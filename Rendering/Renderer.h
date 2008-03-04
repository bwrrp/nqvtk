#pragma once

#include "Camera.h"
#include "Renderable.h"
#include <vector>
#include <QTime>

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLRenderTexture2DRectangle.h"
#include "GLBlaat/GLTexture.h"

namespace NQVTK 
{
	class Renderer
	{
	public:
		Renderer() : camera(0), fbo(0) { };
		virtual ~Renderer() 
		{ 
			DeleteAllRenderables();
			if (camera) delete camera;
		}

		virtual bool Initialize()
		{
			glClearColor(0.2f, 0.3f, 0.5f, 0.0f);

			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			if (camera) delete camera;
			camera = new Camera();

			if (fbo) delete fbo;
			fbo = 0;

			return true;
		}

		virtual void Resize(int w, int h)
		{
			glViewport(0, 0, static_cast<GLint>(w), static_cast<GLint>(h));
			camera->aspect = static_cast<double>(w) / static_cast<double>(h);

			if (!fbo) 
			{
				fbo = GLFramebuffer::New(w, h);
				fbo->CreateDepthTexture();
				GLRendertarget *rt = GLRenderTexture2D::New(w, h, GL_RGBA8, 
					GL_RGBA, GL_UNSIGNED_BYTE);
				fbo->AttachRendertarget(GL_COLOR_ATTACHMENT0_EXT, rt);
				if (!fbo->IsOk())
				{
					qDebug("WARNING! fbo not ok!");
				}
			}
			else
			{
				if (!fbo->Resize(w, h))
				{
					qDebug("WARNING! fbo resize failed!");
					delete fbo;
					fbo = 0;
					// Recurse to create a new FBO instead
					Resize(w, h);
				}
			}
			fbo->Unbind();
		}

		virtual void Clear()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		virtual void Draw()
		{
			// TODO: replace this, add camera reset to focus on all renderables
			Renderable *renderable = renderables[0];
			// TODO: add a useful way to focus camera on objects
			camera->FocusOn(renderable);

			// Set up the camera (matrices)
			camera->Draw();

			// Add some silly rotation while we don't have interactive camera control
			QTime midnight = QTime(0, 0);
			glRotated(static_cast<double>(QTime::currentTime().msecsTo(midnight)) / 30.0, 
				0.0, 1.0, 0.0);

			// DrawRenderables();

			// Simple FBO test
			fbo->Bind();
			Clear();
			DrawRenderables();
			fbo->Unbind();

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			GLTexture *tex = fbo->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
			tex->BindToCurrent();
			glEnable(tex->GetTextureTarget());
			glDisable(GL_LIGHTING);
			glBegin(GL_QUADS);
			glColor3d(1.0, 1.0, 1.0);
			glTexCoord2d(0.0, 1.0);
			glVertex3d(0.0, 0.0, 0.0);
			glTexCoord2d(1.0, 1.0);
			glVertex3d(1.0, 0.0, 0.0);
			glTexCoord2d(1.0, 0.0);
			glVertex3d(1.0, 1.0, 0.0);
			glTexCoord2d(0.0, 0.0);
			glVertex3d(0.0, 1.0, 0.0);
			glEnd();
			glDisable(tex->GetTextureTarget());
			tex->UnbindCurrent();

			tex = fbo->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT);
			tex->BindToCurrent();
			glEnable(tex->GetTextureTarget());
			glDisable(GL_LIGHTING);
			glBegin(GL_QUADS);
			glColor3d(1.0, 1.0, 1.0);
			glTexCoord2d(0.0, 1.0);
			glVertex3d(0.0, -1.0, 0.0);
			glTexCoord2d(1.0, 1.0);
			glVertex3d(1.0, -1.0, 0.0);
			glTexCoord2d(1.0, 0.0);
			glVertex3d(1.0, 0.0, 0.0);
			glTexCoord2d(0.0, 0.0);
			glVertex3d(0.0, 0.0, 0.0);
			glEnd();
			glDisable(tex->GetTextureTarget());
			tex->UnbindCurrent();
		}

		virtual void DrawRenderables()
		{
			// Iterate over all renderables and draw them
			for (std::vector<Renderable*>::const_iterator it = renderables.begin();
				it != renderables.end(); ++it)
			{
				if ((*it)->visible)
				{
					(*it)->Draw();
				}
			}
		}

		void AddRenderable(Renderable *obj)
		{
			if (obj) renderables.push_back(obj);
		}

		void DeleteAllRenderables()
		{
			for (std::vector<Renderable*>::iterator it = renderables.begin();
				it != renderables.end(); ++it)
			{
				delete *it;
			}
			renderables.clear();
		}

	protected:
		Camera *camera;
		std::vector<Renderable*> renderables;

		GLFramebuffer *fbo;

	private:
		// Not implemented
		Renderer(const Renderer&);
		void operator=(const Renderer&);
	};
}
