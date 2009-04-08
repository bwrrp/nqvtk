#pragma once

#include "OverlayRenderer.h"

#include "GLBlaat/GLUtility.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTexture.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	OverlayRenderer::OverlayRenderer(Renderer *base, Renderer *overlay)
		: NestedRenderer(base)
	{
		overlayRenderer = overlay;

		baseFbo = overlayFbo = 0;

		updateBase = updateOverlay = true;
	}

	// ------------------------------------------------------------------------
	OverlayRenderer::~OverlayRenderer()
	{
		if (baseRenderer) delete baseRenderer;
		if (overlayRenderer) delete overlayRenderer;
		if (baseFbo) delete baseFbo;
		if (overlayFbo) delete overlayFbo;
	}

	// ------------------------------------------------------------------------
	bool OverlayRenderer::Initialize()
	{
		bool ok = Superclass::Initialize();
		if (ok && baseRenderer) ok = baseRenderer->Initialize();
		if (ok && overlayRenderer) ok = overlayRenderer->Initialize();

		if (baseFbo) delete baseFbo;
		if (overlayFbo) delete overlayFbo;
		baseFbo = overlayFbo = 0;

		return ok;
	}

	// ------------------------------------------------------------------------
	void OverlayRenderer::SetViewport(int x, int y, int w, int h)
	{
		Superclass::SetViewport(x, y, w, h);
		if (baseRenderer) baseRenderer->SetViewport(x, y, w, h);
		if (overlayRenderer) overlayRenderer->SetViewport(x, y, w, h);

		// Create (and assign) or resize FBOs
		if (baseRenderer)
		{
			if (!baseFbo) 
			{
				baseFbo = GLFramebuffer::New(w, h);
				assert(baseFbo);
				baseFbo->CreateColorTexture();
				baseFbo->CreateDepthBuffer();
				if (!baseFbo->IsOk()) 
				{
					std::cerr << "WARNING! baseFbo not ok!" << std::endl;
				}
				baseFbo->Unbind();
				baseRenderer->SetTarget(baseFbo);
			}
			else
			{
				if (!baseFbo->Resize(w, h))
				{
					std::cerr << "WARNING! baseFbo resize failed!" << std::endl;
				}
			}
		}

		if (overlayRenderer)
		{
			if (!overlayFbo) 
			{
				overlayFbo = GLFramebuffer::New(w, h);
				assert(overlayFbo);
				overlayFbo->CreateColorTexture();
				overlayFbo->CreateDepthBuffer();
				if (!overlayFbo->IsOk()) 
				{
					std::cerr << "WARNING! overlayFbo not ok!" << std::endl;
				}
				overlayFbo->Unbind();
				overlayRenderer->SetTarget(overlayFbo);
			}
			else
			{
				if (!overlayFbo->Resize(w, h))
				{
					std::cerr << "WARNING! overlayFbo resize failed!" << std::endl;
				}
			}
		}
	}

	// ------------------------------------------------------------------------
	void OverlayRenderer::DrawTexture(GLTexture *tex)
	{
		if (!tex) return;
		tex->BindToCurrent();
		glEnable(tex->GetTextureTarget());
		if (tex->GetTextureTarget() == GL_TEXTURE_RECTANGLE_ARB)
		{
			glBegin(GL_QUADS);
			glTexCoord2d(0.0, 0.0);
			glVertex3d(-1.0, -1.0, 0.0);
			glTexCoord2d(tex->GetWidth(), 0.0);
			glVertex3d(1.0, -1.0, 0.0);
			glTexCoord2d(tex->GetWidth(), tex->GetHeight());
			glVertex3d(1.0, 1.0, 0.0);
			glTexCoord2d(0.0, tex->GetHeight());
			glVertex3d(-1.0, 1.0, 0.0);
			glEnd();
		}
		else 
		{
			glBegin(GL_QUADS);
			glTexCoord2d(0.0, 0.0);
			glVertex3d(-1.0, -1.0, 0.0);
			glTexCoord2d(1.0, 0.0);
			glVertex3d(1.0, -1.0, 0.0);
			glTexCoord2d(1.0, 1.0);
			glVertex3d(1.0, 1.0, 0.0);
			glTexCoord2d(0.0, 1.0);
			glVertex3d(-1.0, 1.0, 0.0);
			glEnd();
		}
		glDisable(tex->GetTextureTarget());
		tex->UnbindCurrent();
	}

	// ------------------------------------------------------------------------
	void OverlayRenderer::Draw()
	{
		UpdateCamera();

		// Draw baseRenderer to fbo
		if (baseRenderer && updateBase) 
			baseRenderer->Draw();
		
		// Draw overlayRenderer to fbo
		if (overlayRenderer && updateOverlay) 
			overlayRenderer->Draw();

		// Prepare for rendering
		if (fboTarget)
		{
			fboTarget->Bind();
		}
		else
		{
			glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
		}

		// Composite results
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glColor3d(1.0, 1.0, 1.0);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		Clear();
		if (baseFbo)
		{
			glDisable(GL_BLEND);
			DrawTexture(baseFbo->GetTexture2D());
		}
		if (overlayFbo)
		{
			glEnable(GL_BLEND);
			DrawTexture(overlayFbo->GetTexture2D());
		}
		glPopAttrib();

		GLUtility::CheckOpenGLError("OverlayRenderer::Draw()");

		if (fboTarget)
		{
			fboTarget->Unbind();
		}
	}

	// ------------------------------------------------------------------------
	GLTexture *OverlayRenderer::GetBaseImage()
	{
		return baseFbo->GetTexture2D();
	}

	// ------------------------------------------------------------------------
	GLTexture *OverlayRenderer::GetOverlayImage()
	{
		return overlayFbo->GetTexture2D();
	}
}
