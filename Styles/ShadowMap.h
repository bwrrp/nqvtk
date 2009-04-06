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

			ShadowMap(NQVTK::RenderStyle *baseStyle) 
			{
				this->baseStyle = baseStyle;
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
				if (!fbo->IsOk()) 
				{
					std::cerr << "WARNING! fbo not ok!" << std::endl;
				}
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
				if (!fbo->IsOk()) 
				{
					std::cerr << "WARNING! shadow buffer fbo not ok!" << std::endl;
				}
				fbo->Unbind();

				return fbo;
			}

			virtual GLProgram *CreateScribe()
			{
				baseStyle->SetOption("NQVTK_GENERATE_SHADOWMAP");
				GLProgram *scribe = baseStyle->CreateScribe();
				baseStyle->UnsetOption("NQVTK_GENERATE_SHADOWMAP");
				return scribe;
			}

			virtual GLProgram *CreatePainter()
			{
				baseStyle->SetOption("NQVTK_GENERATE_SHADOWMAP");
				GLProgram *painter = baseStyle->CreatePainter();
				baseStyle->UnsetOption("NQVTK_GENERATE_SHADOWMAP");
				return painter;
			}

			virtual void PrepareForObject(GLProgram *scribe, 
				int objectId, Renderable *renderable)
			{
				baseStyle->PrepareForObject(scribe, objectId, renderable);
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

				// HACK: register the distance field texture
				// TODO: make other styles handle the textures used for shadow mapping
				tm->AddTexture("distanceField", 0, false);
			}

			virtual void UpdateScribeParameters(GLProgram *scribe) 
			{
				baseStyle->UpdateScribeParameters(scribe);
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

			virtual void UpdatePainterParameters(GLProgram *painter)
			{
				baseStyle->UpdatePainterParameters(painter);
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

		protected:
			// Base style
			NQVTK::RenderStyle *baseStyle;

		private:
			// Not implemented
			ShadowMap(const ShadowMap&);
			void operator=(const ShadowMap&);
		};
	}
}
