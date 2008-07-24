#pragma once

#include "Renderer.h"

#include "GLBlaat/GLUtility.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTexture.h"

#include <QObject> // for qDebug
#include <cassert>


namespace NQVTK
{
	class OverlayRenderer : public NQVTK::Renderer
	{
	public:
		typedef Renderer Superclass;

		OverlayRenderer(NQVTK::Renderer *base, NQVTK::Renderer *overlay)
		{
			baseRenderer = base;
			overlayRenderer = overlay;

			baseFbo = overlayFbo = 0;
		}

		virtual ~OverlayRenderer()
		{
			if (baseRenderer) delete baseRenderer;
			if (overlayRenderer) delete overlayRenderer;
			if (baseFbo) delete baseFbo;
			if (overlayFbo) delete overlayFbo;
		}

		virtual bool Initialize()
		{
			bool ok = Superclass::Initialize();
			if (ok && baseRenderer) ok = baseRenderer->Initialize();
			if (ok && overlayRenderer) ok = overlayRenderer->Initialize();

			if (baseFbo) delete baseFbo;
			if (overlayFbo) delete overlayFbo;
			baseFbo = overlayFbo = 0;

			return ok;
		}

		virtual void Resize(int w, int h)
		{
			Superclass::Resize(w, h);
			if (baseRenderer) baseRenderer->Resize(w, h);
			if (overlayRenderer) overlayRenderer->Resize(w, h);

			// Create (and assign) or resize FBOs
			if (baseRenderer)
			{
				if (!baseFbo) 
				{
					baseFbo = GLFramebuffer::New(w, h);
					assert(baseFbo);
					baseFbo->CreateColorTexture();
					baseFbo->CreateDepthBuffer();
					if (!baseFbo->IsOk()) qDebug("WARNING! baseFbo not ok!");
					baseFbo->Unbind();
					baseRenderer->SetTarget(baseFbo);
				}
				else
				{
					if (!baseFbo->Resize(w, h)) qDebug("WARNING! baseFbo resize failed!");
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
					if (!overlayFbo->IsOk()) qDebug("WARNING! overlayFbo not ok!");
					overlayFbo->Unbind();
					overlayRenderer->SetTarget(overlayFbo);
				}
				else
				{
					if (!overlayFbo->Resize(w, h)) qDebug("WARNING! overlayFbo resize failed!");
				}
			}
		}

		void DrawTexture(GLTexture *tex)
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

		virtual void Draw()
		{
			// Draw baseRenderer to fbo
			if (baseRenderer) baseRenderer->Draw();
			
			// Draw overlayRenderer to fbo
			if (overlayRenderer)
			{
				overlayRenderer->Draw();
			}

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

		NQVTK::Renderer *GetBaseRenderer() { return baseRenderer; }
		NQVTK::Renderer *GetOverlayRenderer() { return overlayRenderer; }

	protected:
		NQVTK::Renderer *baseRenderer;
		NQVTK::Renderer *overlayRenderer;

		GLFramebuffer *baseFbo;
		GLFramebuffer *overlayFbo;

	private:
		// Not implemented
		OverlayRenderer(const OverlayRenderer&);
		void operator=(const OverlayRenderer&);
	};
}
