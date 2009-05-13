#pragma once

#include "LayeredRaycaster.h"

#include "ParamSets/VolumeParamSet.h"
#include "Rendering/Volume.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLUtility.h"

#include "Shaders.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	namespace Styles
	{
		// --------------------------------------------------------------------
		LayeredRaycaster::LayeredRaycaster()
		{
			testParam = 0.5f;
			isoOpacity = 0.6f;
			occlusionEdgeThreshold = 0.1f;
			cornerEdgeThreshold = 0.0f;
		}

		// --------------------------------------------------------------------
		GLFramebuffer *LayeredRaycaster::CreateFBO(int w, int h)
		{
			GLFramebuffer *fbo = GLFramebuffer::New(w, h);
			fbo->CreateDepthTextureRectangle();
			int nBufs = 3;
			GLenum bufs[] = {
				GL_COLOR_ATTACHMENT0_EXT, // Positions, in-out buffer
				GL_COLOR_ATTACHMENT1_EXT, // Normals / gradients, object IDs
				GL_COLOR_ATTACHMENT2_EXT  // Colors, opacities
			};
			for (int i = 0; i < nBufs; ++i)
			{
				fbo->CreateColorTextureRectangle(
					bufs[i], GL_RGBA16F_ARB, GL_RGBA, GL_FLOAT);
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
		// Scribe stage peeling pass
		GLProgram *LayeredRaycaster::CreateScribe()
		{
			GLProgram *scribe = GLProgram::New();
			// Scribe vertex shaders
			bool res = scribe->AddVertexShader(
				AddShaderDefines(Shaders::LayeredRaycasterScribePeelVS));
			// Scribe fragment shaders
			if (res) res = scribe->AddFragmentShader(
				AddShaderDefines(Shaders::LayeredRaycasterScribePeelFS));
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
		// Scribe stage raycasting pass
		GLProgram *LayeredRaycaster::CreateRaycaster()
		{
			GLProgram *raycaster = GLProgram::New();
			// Raycaster vertex shaders
			bool res = raycaster->AddVertexShader(
				Shaders::GenericPainterVS);
			// Raycaster fragment shaders
			if (res) res = raycaster->AddFragmentShader(
				AddShaderDefines(Shaders::LayeredRaycasterScribeCastFS));
			if (res) res = raycaster->AddFragmentShader(
				AddShaderDefines(Shaders::LibUtility));
			if (res) res = raycaster->Link();
			std::cout << raycaster->GetInfoLogs() << std::endl;
			if (!res) 
			{
				delete raycaster;
				return 0;
			}
			return raycaster;
		}

		// --------------------------------------------------------------------
		// Painter stage
		GLProgram *LayeredRaycaster::CreatePainter()
		{
			GLProgram *painter = GLProgram::New();
			// Painter vertex shaders
			bool res = painter->AddVertexShader(
				Shaders::GenericPainterVS);
			// Painter fragment shaders
			if (res) res = painter->AddFragmentShader(
				AddShaderDefines(Shaders::LayeredRaycasterPainterFS));
			if (res) res = painter->AddFragmentShader(
				AddShaderDefines(Shaders::LibUtility));
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
		// Used for both the painter and the scribe raycaster passes
		void LayeredRaycaster::RegisterPainterTextures(GLFramebuffer *current, 
			GLFramebuffer *previous) 
		{
			// Register infobuffers
			Superclass::RegisterPainterTextures(current, previous);
			
			// Current layer normals and IDs
			GLTexture *normals = current->GetTexture2D(
				GL_COLOR_ATTACHMENT1_EXT);
			assert(normals);
			tm->AddTexture("normals", normals, false);
			// Current layer colors
			GLTexture *colors = current->GetTexture2D(
				GL_COLOR_ATTACHMENT2_EXT);
			assert(colors);
			tm->AddTexture("colors", colors, false);
		}

		// --------------------------------------------------------------------
		void LayeredRaycaster::UpdatePainterParameters(GLProgram *painter)
		{
			Superclass::UpdatePainterParameters(painter);
			painter->SetUniform1f("testParam", testParam);
			painter->SetUniform1f("isoOpacity", isoOpacity);
			painter->SetUniform1f("cornerEdgeThreshold", cornerEdgeThreshold);

			// TODO: add a method to compute maxRayLength over all volumes
			double maxRayLength = 10.0;
			/*
			for (unsigned int i = 0; i < volumes.size(); ++i)
			{
				NQVTK::VolumeParamSet *vps = volumes[i];
				if (vps)
				{
					double w = vps->GetVolume()->GetWidth();
					double h = vps->GetVolume()->GetHeight();
					double d = vps->GetVolume()->GetDepth();
					double diameter = sqrt(w * w + h * h + d * d);
					if (diameter > maxRayLength) maxRayLength = diameter;
				}
			}
			*/
			painter->SetUniform1f("occlusionEdgeThreshold", 
				occlusionEdgeThreshold * 
				static_cast<float>(maxRayLength / 10.0));
		}
	}
}
