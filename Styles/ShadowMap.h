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
		class ShadowMap : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			ShadowMap() 
			{
				useGridTexture = false;
				useGlyphTexture = false;
			}

			virtual ~ShadowMap() { }

			virtual GLFramebuffer *CreateFBO(int w, int h)
			{
				GLFramebuffer *fbo = GLFramebuffer::New(w, h);
				fbo->CreateDepthTextureRectangle();
				int nBufs = 1;
				GLenum bufs[] = {
					GL_COLOR_ATTACHMENT0_EXT
				};
				for (int i = 0; i < nBufs; ++i)
				{
					fbo->CreateColorTextureRectangle(
						bufs[i], GL_RGBA16F_ARB, GL_RGBA, GL_FLOAT);
					GLUtility::SetDefaultColorTextureParameters(
						fbo->GetTexture2D(bufs[i]));
				}
				glDrawBuffers(nBufs, bufs);
				if (!fbo->IsOk()) qDebug("WARNING! fbo not ok!");
				fbo->Unbind();

				return fbo;
			}

			virtual GLFramebuffer *CreateShadowBufferFBO(int w, int h)
			{
				GLFramebuffer *fbo = GLFramebuffer::New(w, h);
				// We only need a color texture to store the shadow map
				fbo->CreateColorTexture(
					GL_COLOR_ATTACHMENT0_EXT, 
					GL_RGBA16F_ARB, GL_RGBA, GL_FLOAT);
				GLTexture *tex = fbo->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				// With variance shadow mapping we can linearly interpolate the texture
				tex->BindToCurrent();
				glTexParameteri(tex->GetTextureTarget(), 
					GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(tex->GetTextureTarget(), 
					GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				tex->UnbindCurrent();
				if (!fbo->IsOk()) qDebug("WARNING! shadow buffer fbo not ok!");
				fbo->Unbind();

				return fbo;
			}

			virtual GLProgram *CreateScribe()
			{
				GLProgram *scribe = GLProgram::New();
				// Scribe vertex shaders
				bool res = scribe->AddVertexShader(
					Shaders::ShadowMapScribeVS);
				// Scribe fragment shaders
				if (res) res = scribe->AddFragmentShader(
					Shaders::ShadowMapScribeFS);
				if (res) res = scribe->AddFragmentShader(
					Shaders::LibUtility);
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
				bool res = painter->AddVertexShader(
					Shaders::GenericPainterVS);
				if (res) res = painter->AddFragmentShader(
					Shaders::ShadowMapPainterFS);
				if (res) res = painter->AddFragmentShader(
					Shaders::LibUtility);
				if (res) res = painter->AddFragmentShader(
					Shaders::LibCSG);
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
				// Get the previous layer's info buffer
				GLTexture *infoBuffer = previous->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(infoBuffer);
				tm->AddTexture("infoBuffer", infoBuffer, false);

				// Get the previous layer's depth buffer
				GLTexture *depthBuffer = previous->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT);
				assert(depthBuffer);
				GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
				glTexParameteri(depthBuffer->GetTextureTarget(), 
					GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
				depthBuffer->UnbindCurrent();
				tm->AddTexture("depthBuffer", depthBuffer, false);
			}

			virtual void UnregisterScribeTextures() 
			{
				//tm->RemoveTexture("infoBuffer");
				//tm->RemoveTexture("depthBuffer");
			}

			virtual void RegisterPainterTextures(GLFramebuffer *current, GLFramebuffer *previous) 
			{
				// Previous layer info buffer
				GLTexture *infoPrevious = previous->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(infoPrevious);
				tm->AddTexture("infoPrevious", infoPrevious, false);
				// Current layer info buffer
				GLTexture *infoCurrent = current->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(infoCurrent);
				tm->AddTexture("infoCurrent", infoCurrent, false);
			}

			virtual void UnregisterPainterTextures() 
			{
				//tm->RemoveTexture("infoPrevious");
				//tm->RemoveTexture("infoCurrent");
			}

			virtual void UpdatePainterParameters(GLProgram *scribe)
			{
				scribe->SetUniform1i("useGridTexture", useGridTexture);
				scribe->SetUniform1i("useGlyphTexture", useGlyphTexture);
			}

			virtual void DrawBackground()
			{
				glDisable(GL_LIGHTING);
				glEnable(GL_BLEND);

				glBegin(GL_QUADS);
				glColor4d(1.0, 1.0, 0.0, 1.0);
				glVertex3d(-1.0, -1.0, 0.0);
				glVertex3d(1.0, -1.0, 0.0);
				glVertex3d(1.0, 1.0, 0.0);
				glVertex3d(-1.0, 1.0, 0.0);
				glEnd();

				glDisable(GL_BLEND);
			}

			// Program parameters
			// - Painter
			bool useGridTexture;
			bool useGlyphTexture;

		private:
			// Not implemented
			ShadowMap(const ShadowMap&);
			void operator=(const ShadowMap&);
		};
	}
}
