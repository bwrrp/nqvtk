#pragma once

#include "Camera.h"
#include "Renderable.h"
#include <vector>
#include <QTime>

namespace NQVTK 
{
	class Renderer
	{
	public:
		Renderer() : camera(0) { };
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

			return true;
		}

		virtual void Resize(int w, int h)
		{
			glViewport(0, 0, static_cast<GLint>(w), static_cast<GLint>(h));
			camera->aspect = static_cast<double>(w) / static_cast<double>(h);
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

			// Add some silly rotation
			// TODO: local transformations should be handled by the Renderable
			QTime midnight = QTime(0, 0);
			glRotated(static_cast<double>(QTime::currentTime().msecsTo(midnight)) / 30.0, 
				0.0, 1.0, 0.0);
			Vector3 center = renderable->GetCenter();
			glTranslated(-center.x, -center.y, -center.z);

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

	private:
		// Not implemented
		Renderer(const Renderer&);
		void operator=(const Renderer&);
	};
}
