#pragma once

#include "ShadowMap.h"

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
		ShadowMap::ShadowMap(NQVTK::RenderStyle *baseStyle) 
		{
			this->baseStyle = baseStyle;
		}

		// --------------------------------------------------------------------
		ShadowMap::~ShadowMap()
		{
		}

		// --------------------------------------------------------------------
		GLFramebuffer *ShadowMap::CreateFBO(int w, int h)
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

		// --------------------------------------------------------------------
		GLFramebuffer *ShadowMap::CreateShadowBufferFBO(int w, int h)
		{
			GLFramebuffer *fbo = GLFramebuffer::New(w, h);
			// We only need a color texture to store the shadow map
			fbo->CreateColorTexture(
				GL_COLOR_ATTACHMENT0_EXT, 
				GL_RGBA16F_ARB, GL_RGBA, GL_FLOAT);
			GLTexture *tex = fbo->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
			// With variance shadow mapping we can use linear interpolation
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

		// --------------------------------------------------------------------
		GLProgram *ShadowMap::CreateScribe()
		{
			baseStyle->SetOption("NQVTK_GENERATE_SHADOWMAP");
			GLProgram *scribe = baseStyle->CreateScribe();
			baseStyle->UnsetOption("NQVTK_GENERATE_SHADOWMAP");
			return scribe;
		}

		// --------------------------------------------------------------------
		GLProgram *ShadowMap::CreatePainter()
		{
			baseStyle->SetOption("NQVTK_GENERATE_SHADOWMAP");
			GLProgram *painter = baseStyle->CreatePainter();
			baseStyle->UnsetOption("NQVTK_GENERATE_SHADOWMAP");
			return painter;
		}

		// --------------------------------------------------------------------
		void ShadowMap::PrepareForObject(GLProgram *scribe, 
			int objectId, Renderable *renderable)
		{
			baseStyle->PrepareForObject(scribe, objectId, renderable);
		}

		// --------------------------------------------------------------------
		void ShadowMap::RegisterScribeTextures(GLFramebuffer *previous) 
		{
			// Get the previous layer's info buffer
			GLTexture *infoBuffer = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT0_EXT);
			assert(infoBuffer);
			tm->AddTexture("infoBuffer", infoBuffer, false);

			// Get the previous layer's depth buffer
			GLTexture *depthBuffer = previous->GetTexture2D(
				GL_DEPTH_ATTACHMENT_EXT);
			assert(depthBuffer);
			GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
			glTexParameteri(depthBuffer->GetTextureTarget(), 
				GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
			depthBuffer->UnbindCurrent();
			tm->AddTexture("depthBuffer", depthBuffer, false);

			// HACK: register the volume texture
			// TODO: make other styles handle the textures used for shadow mapping
			tm->AddTexture("volume", 0, false);
		}

		// --------------------------------------------------------------------
		void ShadowMap::UpdateScribeParameters(GLProgram *scribe) 
		{
			baseStyle->UpdateScribeParameters(scribe);
		}

		// --------------------------------------------------------------------
		void ShadowMap::RegisterPainterTextures(GLFramebuffer *current, 
			GLFramebuffer *previous) 
		{
			// Previous layer info buffer
			GLTexture *infoPrevious = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT0_EXT);
			assert(infoPrevious);
			tm->AddTexture("infoPrevious", infoPrevious, false);
			// Current layer info buffer
			GLTexture *infoCurrent = current->GetTexture2D(
				GL_COLOR_ATTACHMENT0_EXT);
			assert(infoCurrent);
			tm->AddTexture("infoCurrent", infoCurrent, false);
		}

		// --------------------------------------------------------------------
		void ShadowMap::UpdatePainterParameters(GLProgram *painter)
		{
			baseStyle->UpdatePainterParameters(painter);
		}

		// --------------------------------------------------------------------
		void ShadowMap::DrawBackground()
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
	}
}
