#pragma once

#include "GLBlaat/GL.h"

#include "Camera.h"
#include "Renderable.h"

#include <vector>

#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLUtility.h"

#include <QObject> // for qDebug
#include <cassert>
#include <limits>


namespace NQVTK
{
	class Renderer
	{
	public:
		Renderer() : camera(0), tm(0), fboTarget(0)
		{
			viewportX = 0;
			viewportY = 0;

			lightOffsetDirection = 270.0;
			lightRelativeToCamera = true;
		}

		virtual ~Renderer() 
		{
			DeleteAllRenderables();
			if (camera) delete camera;
			if (tm) delete tm;
		}

		virtual bool Initialize() 
		{
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			// Make sure we have a camera
			GetCamera();

			if (!tm)
			{
				tm = GLTextureManager::New();
				if (!tm)
				{
					qDebug("Failed to create texture manager! Check hardware requirements...");
					return false;
				}
			}
			tm->BeginNewPass();

			return true;
		}

		virtual void Resize(int w, int h)
		{
			this->viewportWidth = w;
			this->viewportHeight = h;

			glViewport(viewportX, viewportY, w, h);
			camera->aspect = static_cast<double>(w) / static_cast<double>(h);

			if (fboTarget)
			{
				if (!fboTarget->Resize(w, h)) qDebug("WARNING! fboTarget resize failed!");
			}
		}

		virtual void Clear()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		void TestDrawTexture(GLTexture *tex, 
			double xmin, double xmax, 
			double ymin, double ymax)
		{
			if (!tex) return;

			glPushAttrib(GL_ALL_ATTRIB_BITS);

			glColor3d(1.0, 1.0, 1.0);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			tex->BindToCurrent();
			glEnable(tex->GetTextureTarget());
			if (tex->GetTextureTarget() == GL_TEXTURE_RECTANGLE_ARB)
			{
				glBegin(GL_QUADS);
				glTexCoord2d(0.0, 0.0);
				glVertex3d(xmin, ymin, 0.0);
				glTexCoord2d(tex->GetWidth(), 0.0);
				glVertex3d(xmax, ymin, 0.0);
				glTexCoord2d(tex->GetWidth(), tex->GetHeight());
				glVertex3d(xmax, ymax, 0.0);
				glTexCoord2d(0.0, tex->GetHeight());
				glVertex3d(xmin, ymax, 0.0);
				glEnd();
			}
			else 
			{
				glBegin(GL_QUADS);
				glTexCoord2d(0.0, 0.0);
				glVertex3d(xmin, ymin, 0.0);
				glTexCoord2d(1.0, 0.0);
				glVertex3d(xmax, ymin, 0.0);
				glTexCoord2d(1.0, 1.0);
				glVertex3d(xmax, ymax, 0.0);
				glTexCoord2d(0.0, 1.0);
				glVertex3d(xmin, ymax, 0.0);
				glEnd();
			}
			glDisable(tex->GetTextureTarget());
			tex->UnbindCurrent();

			glPopAttrib();
		}

		virtual void DrawCamera()
		{
			// Get bounds for all renderables
			const double inf = std::numeric_limits<double>::infinity();
			double bounds[] = {inf, -inf, inf, -inf, inf, -inf};
			unsigned int numRenderables = GetNumberOfRenderables();
			for (unsigned int i = 0; i < numRenderables; ++i)
			{
				Renderable *renderable = GetRenderable(i);
				if (renderable)
				{
					double rbounds[6];
					renderable->GetBounds(rbounds);
					for (int i = 0; i < 3; ++i)
					{
						if (rbounds[i*2] < bounds[i*2]) bounds[i*2] = rbounds[i*2];
						if (rbounds[i*2+1] > bounds[i*2+1]) bounds[i*2+1] = rbounds[i*2+1];
					}
				}
			}

			camera->SetZPlanes(bounds);

			// Set up the camera (matrices)
			camera->Draw();
		}

		virtual void UpdateLighting()
		{
			if (lightRelativeToCamera)
			{
				camera->Update();
				const double DEGREES_TO_RADIANS = 0.0174532925199433;
				Vector3 viewDir = (camera->focus - camera->position);
				Vector3 sideDir = viewDir.cross(camera->up).normalized();
				Vector3 upDir = sideDir.cross(viewDir).normalized();
				Vector3 offset = -sin(lightOffsetDirection * DEGREES_TO_RADIANS) * sideDir - 
					cos(lightOffsetDirection * DEGREES_TO_RADIANS) * upDir;
				offset *= viewDir.length() / 2.0;
				lightPos = camera->position + offset;
			}

			// Update OpenGL light direction
			Vector3 lightDir = (lightPos - camera->focus).normalized();
			float ldir[] = {lightDir.x, lightDir.y, lightDir.z, 0.0};
			glLightfv(GL_LIGHT0, GL_POSITION, ldir);
		}

		// To be implemented in derived classes
		virtual void Draw() = 0;

		// Hook for per-renderable processing
		virtual void PrepareForRenderable(int objectId, NQVTK::Renderable *renderable) { }

		// Loop over and draw all renderables
		virtual void DrawRenderables()
		{
			int objectId = 0;
			// Iterate over all renderables and draw them
			for (std::vector<Renderable*>::const_iterator it = renderables.begin();
				it != renderables.end(); ++it)
			{
				Renderable *renderable = *it;
				if (renderable)
				{
					if (renderable->visible)
					{
						PrepareForRenderable(objectId, renderable);
						renderable->Draw();
					}
				}
				++objectId;
			}
		}

		int AddRenderable(Renderable *obj)
		{
			renderables.push_back(obj);
			return renderables.size() - 1;
		}

		Renderable *GetRenderable(unsigned int i)
		{
			if (i >= renderables.size()) return 0;
			return renderables[i];
		}

		Renderable *SetRenderable(unsigned int i, Renderable *obj)
		{
			assert(i < static_cast<unsigned int>(GetNumberOfRenderables()));
			Renderable *old = GetRenderable(i);
			renderables[i] = obj;
			return old;
		}

		int GetNumberOfRenderables() 
		{
			return renderables.size();
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

		void SetRenderables(std::vector<Renderable*> renderables)
		{
			// Replace the renderables with the given set
			// Beware of memory leaks!
			this->renderables = renderables;
		}

		void ResetRenderables()
		{
			for (std::vector<Renderable*>::iterator it = renderables.begin();
				it != renderables.end(); ++it)
			{
				(*it)->position = Vector3();
				(*it)->rotateX = 0.0;
				(*it)->rotateY = 0.0;
			}
		}

		Camera *GetCamera() 
		{
			// Create a default camera if we don't have one
			if (!camera) 
			{
				camera = new Camera();
			}

			return camera; 
		}

		void SetCamera(Camera *cam)
		{
			if (camera) delete camera;
			camera = cam;
		}

		void ShareTextures(GLTextureManager *tm)
		{
			this->tm = tm;
		}

		GLFramebuffer *SetTarget(GLFramebuffer *target)
		{
			GLFramebuffer *oldTarget = this->fboTarget;
			this->fboTarget = target;
			if (target)
			{
				// Make sure it's the right size
				bool ok = target->Resize(viewportWidth, viewportHeight);
				assert(ok);
			}
			return oldTarget;
		}

		GLFramebuffer *GetTarget() { return this->fboTarget; }

		int GetWidth() { return this->viewportWidth; }
		int GetHeight() { return this->viewportHeight; }

		// TODO: should probably be made into properties
		double lightOffsetDirection;
		bool lightRelativeToCamera;

	protected:
		// Area to draw in
		int viewportX;
		int viewportY;
		int viewportWidth;
		int viewportHeight;

		Camera *camera;
		std::vector<Renderable*> renderables;
		NQVTK::Vector3 lightPos;

		GLTextureManager *tm;

		GLFramebuffer *fboTarget;

	private:
		// Not implemented
		Renderer(const Renderer&);
		void operator=(const Renderer&);
	};
}
