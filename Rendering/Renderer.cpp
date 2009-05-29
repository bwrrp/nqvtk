#pragma once

#include "Renderer.h"
#include "Camera.h"
#include "View.h"
#include "Renderables/Renderable.h"
#include "Math/Vector3.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLUtility.h"

#include <cassert>
#include <iostream>
#include <limits>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	Renderer::Renderer() : camera(0), view(0), tm(0), fboTarget(0)
	{
		viewportX = 0;
		viewportY = 0;
		viewportWidth = 1;
		viewportHeight = 1;

		lightOffsetDirection = 270.0;
		lightRelativeToCamera = true;
	}

	// ------------------------------------------------------------------------
	Renderer::~Renderer() 
	{
		delete camera;
		delete view;
		delete tm;
	}

	// ------------------------------------------------------------------------
	bool Renderer::Initialize()
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
				std::cerr << "Failed to create texture manager! " <<
					"Check hardware requirements..." << std::endl;
				return false;
			}
		}
		tm->BeginNewPass();

		return true;
	}

	// ------------------------------------------------------------------------
	void Renderer::Move(int x, int y)
	{
		SetViewport(x, y, viewportWidth, viewportHeight);
	}

	// ------------------------------------------------------------------------
	void Renderer::Resize(int w, int h)
	{
		SetViewport(viewportX, viewportY, w, h);
	}

	// ------------------------------------------------------------------------
	void Renderer::SetViewport(int x, int y, int w, int h)
	{
		viewportX = x;
		viewportY = y;
		viewportWidth = w;
		viewportHeight = h;

		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
		GetCamera()->aspect = static_cast<double>(w) / static_cast<double>(h);

		// NOTE: The target is never resized by the renderer it's assigned to, 
		// it should be managed by its owner!
	}

	// ------------------------------------------------------------------------
	void Renderer::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// ------------------------------------------------------------------------
	void Renderer::SetScene(Scene *scene)
	{
		if (view)
		{
			view->SetScene(scene);
			ResetTextures();
		}
		else
		{
			SetView(new View(scene));
		}
	}

	// ------------------------------------------------------------------------
	void Renderer::SetView(View *view)
	{
		if (view == this->view) return;

		delete this->view;
		this->view = view;

		ResetTextures();
	}

	// ------------------------------------------------------------------------
	Camera *Renderer::GetCamera() 
	{
		// Create a default camera if we don't have one
		if (!camera) 
		{
			camera = new Camera();
		}

		return camera; 
	}

	// ------------------------------------------------------------------------
	void Renderer::SetCamera(Camera *cam)
	{
		if (camera) delete camera;
		camera = cam;
	}

	// ------------------------------------------------------------------------
	GLFramebuffer *Renderer::SetTarget(GLFramebuffer *target)
	{
		GLFramebuffer *oldTarget = this->fboTarget;
		this->fboTarget = target;
		return oldTarget;
	}

	// ------------------------------------------------------------------------
	void Renderer::DrawCamera()
	{
		// Get bounds for all visible renderables
		// TODO: this could be moved to the scene or the view
		const double inf = std::numeric_limits<double>::infinity();
		double bounds[] = {inf, -inf, inf, -inf, inf, -inf};
		if (view)
		{
			unsigned int numRenderables = view->GetNumberOfRenderables();
			for (unsigned int i = 0; i < numRenderables; ++i)
			{
				Renderable *renderable = view->GetRenderable(i);
				if (view->GetVisibility(i))
				{
					double rbounds[6];
					renderable->GetBounds(rbounds);
					for (int i = 0; i < 3; ++i)
					{
						if (rbounds[i*2] < bounds[i*2]) 
							bounds[i*2] = rbounds[i*2];

						if (rbounds[i*2+1] > bounds[i*2+1]) 
							bounds[i*2+1] = rbounds[i*2+1];
					}
				}
			}
		}

		camera->SetZPlanes(bounds);

		// Set up the camera (matrices)
		camera->Draw();
	}

	// ------------------------------------------------------------------------
	void Renderer::ResetTextures()
	{
		// Reset texture manager binding cache
		tm->BeginNewPass();
	}

	// ------------------------------------------------------------------------
	void Renderer::UpdateLighting()
	{
		if (lightRelativeToCamera)
		{
			camera->Update();
			const double DEGREES_TO_RADIANS = 0.0174532925199433;
			Vector3 viewDir = (camera->focus - camera->position);
			Vector3 sideDir = viewDir.cross(camera->up).normalized();
			Vector3 upDir = sideDir.cross(viewDir).normalized();
			Vector3 offset = 
				-sin(lightOffsetDirection * DEGREES_TO_RADIANS) * sideDir - 
				cos(lightOffsetDirection * DEGREES_TO_RADIANS) * upDir;
			offset *= viewDir.length() / 2.0;
			lightPos = camera->position + offset;
		}

		// Update OpenGL light direction
		Vector3 lightDir = (lightPos - camera->focus).normalized();
		float ldir[] = {
			static_cast<float>(lightDir.x), 
			static_cast<float>(lightDir.y), 
			static_cast<float>(lightDir.z), 
			0.0f};
		glLightfv(GL_LIGHT0, GL_POSITION, ldir);
	}

	// ------------------------------------------------------------------------
	void Renderer::PrepareForRenderable(int objectId, Renderable *renderable)
	{
	}

	// ------------------------------------------------------------------------
	void Renderer::DrawRenderables()
	{
		if (!view) return;

		// Iterate over all renderables in the scene and draw them
		for (int objectId = 0; objectId < view->GetNumberOfRenderables(); 
			++objectId)
		{
			// Visibility implies that the renderable is not null
			if (view->GetVisibility(objectId))
			{
				Renderable *renderable = view->GetRenderable(objectId);
				PrepareForRenderable(objectId, renderable);
				renderable->Draw();
			}
		}
	}

	// ------------------------------------------------------------------------
	void Renderer::DrawTexture(GLTexture *tex, 
			double xmin, double xmax, double ymin, double ymax)
	{
		if (!tex) return;

		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glColor3d(1.0, 1.0, 1.0);
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
}
