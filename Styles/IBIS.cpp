#pragma once

#include "IBIS.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLUtility.h"

#include "Rendering/View.h"
#include "Renderables/Renderable.h"
#include "ParamSets/PCAParamSet.h"

#include "Shaders.h"

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

namespace NQVTK
{
	namespace Styles
	{
		// --------------------------------------------------------------------
		IBIS::IBIS()
		{ 
			// Set default parameters
			useContours = true;
			useFatContours = false;
			contourDepthEpsilon = 0.005f;
			useFog = true;
			depthCueRange = 10.0f;
			pvalueThreshold = 1.0;

			SetOption("NQVTK_USE_PVALS");
		}

		// --------------------------------------------------------------------
		IBIS::~IBIS()
		{
		}

		// --------------------------------------------------------------------
		GLFramebuffer *IBIS::CreateFBO(int w, int h)
		{
			GLFramebuffer *fbo = GLFramebuffer::New(w, h);
			fbo->CreateDepthTextureRectangle();
			int nBufs = 3;
			GLenum bufs[] = {
				GL_COLOR_ATTACHMENT0_EXT, 
				GL_COLOR_ATTACHMENT1_EXT, 
				GL_COLOR_ATTACHMENT2_EXT
			};
			for (int i = 0; i < nBufs; ++i)
			{
				fbo->CreateColorTextureRectangle(bufs[i]);
				GLUtility::SetDefaultColorTextureParameters(
					fbo->GetTexture2D(bufs[i]));
			}
			glDrawBuffers(nBufs, bufs);
			if (!fbo->IsOk()) 
			{
				std::cerr << "WARNING! fbo not ok!" << std::endl;
			}
			fbo->Unbind();

			return fbo;
		}

		// --------------------------------------------------------------------
		GLProgram *IBIS::CreateScribe()
		{
			GLProgram *scribe = GLProgram::New();
			// Scribe vertex shaders
			bool res = scribe->AddVertexShader(
				AddShaderDefines(Shaders::CommonIBISScribeVS));
			// Scribe fragment shaders
			if (res) res = scribe->AddFragmentShader(
				AddShaderDefines(Shaders::CommonIBISScribeFS));
			if (res) res = scribe->AddFragmentShader(
				AddShaderDefines(Shaders::LibUtility));
			if (res) res = scribe->Link();
			std::cout << scribe->GetInfoLogs() << std::endl;
			if (!res)
			{
				delete scribe;
				return 0;
			}
			return scribe;
		}

		// --------------------------------------------------------------------
		GLProgram *IBIS::CreatePainter()
		{
			GLProgram *painter = GLProgram::New();
			// Painter vertex shaders
			bool res = painter->AddVertexShader(
				Shaders::GenericPainterVS);
			// Painter fragment shaders
			if (res) res = painter->AddFragmentShader(
				AddShaderDefines(Shaders::CommonIBISPainterFS));
			if (res) res = painter->AddFragmentShader(
				AddShaderDefines(Shaders::LibUtility));
			if (res) res = painter->AddFragmentShader(
				AddShaderDefines(Shaders::LibCSG));
			if (res) res = painter->Link();
			std::cout << painter->GetInfoLogs() << std::endl;
			if (!res) 
			{
				delete painter;
				return 0;
			}
			return painter;
		}

		// --------------------------------------------------------------------
		void IBIS::RegisterScribeTextures(GLFramebuffer *previous)
		{
			// Get the previous layer's depth buffer
			GLTexture *depthBuffer = previous->GetTexture2D(
				GL_DEPTH_ATTACHMENT_EXT);
			assert(depthBuffer);
			GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
			glTexParameteri(depthBuffer->GetTextureTarget(), 
				GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
			depthBuffer->UnbindCurrent();
			tm->AddTexture("depthBuffer", depthBuffer, false);

			// Get the previous layer's info buffer
			GLTexture *infoBuffer = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT2_EXT);
			assert(infoBuffer);
			tm->AddTexture("infoBuffer", infoBuffer, false);
		}

		// --------------------------------------------------------------------
		void IBIS::RegisterPainterTextures(GLFramebuffer *current, 
			GLFramebuffer *previous) 
		{
			// Previous layer info buffer
			GLTexture *infoPrevious = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT2_EXT);
			assert(infoPrevious);
			tm->AddTexture("infoPrevious", infoPrevious, false);
			// Current layer info buffer
			GLTexture *infoCurrent = current->GetTexture2D(
				GL_COLOR_ATTACHMENT2_EXT);
			assert(infoCurrent);
			tm->AddTexture("infoCurrent", infoCurrent, false);
			// Current layer colors
			GLTexture *colors = current->GetTexture2D(
				GL_COLOR_ATTACHMENT0_EXT);
			assert(colors);
			tm->AddTexture("colors", colors, false);
			// Current layer normals
			GLTexture *normals = current->GetTexture2D(
				GL_COLOR_ATTACHMENT1_EXT);
			assert(normals);
			tm->AddTexture("normals", normals, false);
		}

		// --------------------------------------------------------------------
		void IBIS::UpdatePainterParameters(GLProgram *painter)
		{
			// Set program parameters
			painter->SetUniform1i("useContours", useContours);
			painter->SetUniform1i("useFatContours", useFatContours);
			painter->SetUniform1f("contourDepthEpsilon", contourDepthEpsilon);
			painter->SetUniform1i("useFog", useFog);
			painter->SetUniform1f("depthCueRange", depthCueRange);
			painter->SetUniform1i("clipId", clipId);
		}

		// --------------------------------------------------------------------
		void IBIS::UpdateScribeParameters(GLProgram *scribe)
		{
			// Set program parameters
			scribe->SetUniform1f("pvalueThreshold", pvalueThreshold);
		}

		// --------------------------------------------------------------------
		void IBIS::SceneChanged(View *view)
		{
			// Find maximum number of PCA eigenmodes
			unsigned int maxNumEigenModes = 0;
			for (unsigned int i = 0; i < view->GetNumberOfRenderables(); ++i)
			{
				Renderable *renderable = view->GetRenderable(i);
				if (renderable)
				{
					PCAParamSet *pcaps = dynamic_cast<PCAParamSet*>(
						renderable->GetParamSet("pca"));
					if (pcaps)
					{
						unsigned int numEigenModes = pcaps->weights.size();
						if (numEigenModes > maxNumEigenModes)
						{
							maxNumEigenModes = numEigenModes;
						}
					}
				}
			}
			SetOption("NQVTK_USE_PCA", maxNumEigenModes);
		}
	}
}
