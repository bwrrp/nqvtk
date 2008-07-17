#pragma once

#include "GLBlaat/GL.h"

#include "Camera.h"
#include "Renderable.h"

#include <vector>

#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLUtility.h"

#include <QObject> // for qDebug
#include <cassert>


namespace NQVTK
{
	class Renderer
	{
	public:
		Renderer() : camera(0) 
		{
			viewportX = 0;
			viewportY = 0;
		}

		virtual ~Renderer() 
		{
			DeleteAllRenderables();
			if (camera) delete camera;
		}

		virtual bool Initialize() 
		{
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);

			if (!camera) 
			{
				camera = new Camera();
			}

			return true;
		}

		virtual void Resize(int w, int h)
		{
			this->viewportWidth = w;
			this->viewportHeight = h;

			glViewport(viewportX, viewportY, w, h);
			camera->aspect = static_cast<double>(w) / static_cast<double>(h);
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
			glColor3d(1.0, 1.0, 1.0);
			glDisable(GL_BLEND);
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
		}

		virtual void DrawCamera()
		{
			// TODO: replace this, add camera reset to focus on all renderables
			Renderable *renderable = renderables[0];
			camera->FocusOn(renderable);

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
				Vector3 offset = -sin(lightOffsetDirection * DEGREES_TO_RADIANS) * sideDir - 
					cos(lightOffsetDirection * DEGREES_TO_RADIANS) * camera->up;
				offset *= viewDir.length() / 2.0;
				lightPos = camera->position + offset;
			}

			// Update OpenGL light position
			float lpos[] = {lightPos.x, lightPos.y, lightPos.z, 0.0};
			glLightfv(GL_LIGHT0, GL_POSITION, lpos);
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
				if ((*it)->visible)
				{
					PrepareForRenderable(objectId, *it);
					(*it)->Draw();
				}
				++objectId;
			}
		}

		void AddRenderable(Renderable *obj)
		{
			if (obj) renderables.push_back(obj);
		}

		Renderable *GetRenderable(unsigned int i)
		{
			if (i >= renderables.size()) return 0;
			return renderables[i];
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

		Camera *GetCamera() { return camera; }

		void SetCamera(Camera *cam)
		{
			if (camera) delete camera;
			camera = cam;
		}

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
	};
}
