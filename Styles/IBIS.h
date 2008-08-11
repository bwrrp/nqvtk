#pragma once

#include "RenderStyle.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLUtility.h"

#include "Shaders.h"

#include <cassert>

namespace NQVTK
{
	namespace Styles
	{
		class IBIS : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			IBIS()
			{ 
				// Set default parameters
				useFatContours = false;
				contourDepthEpsilon = 0.005f;
				useFog = true;
				depthCueRange = 10.0f;
			}
			
			virtual ~IBIS() { }

			virtual GLFramebuffer *CreateFBO(int w, int h)
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
				if (!fbo->IsOk()) qDebug("WARNING! fbo not ok!");
				fbo->Unbind();

				return fbo;
			}

			virtual GLProgram *CreateScribe()
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
				qDebug(scribe->GetInfoLogs().c_str());
				if (!res)
				{
					delete scribe;
					return 0;
				}
				return scribe;
			}

			virtual GLProgram *CreatePainter()
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
				qDebug(painter->GetInfoLogs().c_str());
				if (!res) 
				{
					delete painter;
					return 0;
				}
				return painter;
			}

			virtual void RegisterScribeTextures(GLFramebuffer *previous) 
			{
				// Get the previous layer's depth buffer
				GLTexture *depthBuffer = previous->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT);
				assert(depthBuffer);
				GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
				glTexParameteri(depthBuffer->GetTextureTarget(), 
					GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
				depthBuffer->UnbindCurrent();
				tm->AddTexture("depthBuffer", depthBuffer, false);

				// Get the previous layer's info buffer
				GLTexture *infoBuffer = previous->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);
				assert(infoBuffer);
				tm->AddTexture("infoBuffer", infoBuffer, false);
			}

			virtual void RegisterPainterTextures(GLFramebuffer *current, GLFramebuffer *previous) 
			{
				// Previous layer info buffer
				GLTexture *infoPrevious = previous->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);
				assert(infoPrevious);
				tm->AddTexture("infoPrevious", infoPrevious, false);
				// Current layer info buffer
				GLTexture *infoCurrent = current->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);
				assert(infoCurrent);
				tm->AddTexture("infoCurrent", infoCurrent, false);
				// Current layer colors
				GLTexture *colors = current->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(colors);
				tm->AddTexture("colors", colors, false);
				// Current layer normals
				GLTexture *normals = current->GetTexture2D(GL_COLOR_ATTACHMENT1_EXT);
				assert(normals);
				tm->AddTexture("normals", normals, false);
			}

			virtual void UpdatePainterParameters(GLProgram *painter)
			{
				// Set program parameters
				painter->SetUniform1i("useFatContours", useFatContours);
				painter->SetUniform1f("contourDepthEpsilon", contourDepthEpsilon);
				painter->SetUniform1i("useFog", useFog);
				painter->SetUniform1f("depthCueRange", depthCueRange);
			}

			// Program parameters
			// - Painter
			bool useFatContours;
			float contourDepthEpsilon;
			bool useFog;
			float depthCueRange;

		private:
			// Not implemented
			IBIS(const IBIS&);
			void operator=(const IBIS&);
		};
	}
}
