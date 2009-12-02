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

namespace NQVTK
{
	// ------------------------------------------------------------------------
	Renderer::Renderer() : camera(0), view(0), tm(0), fboTarget(0)
	{
		viewportX = 0;
		viewportY = 0;
		viewportWidth = -1;
		viewportHeight = -1;

		jitterX = jitterY = 0.0;

		initialized = false;

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
	bool Renderer::TryInitialize()
	{
		initialized = Initialize();
		// If this is re-initialization, pretend we're resizing as well
		if (viewportWidth > 0 && viewportHeight > 0)
		{
			Resize(viewportWidth, viewportHeight);
		}
		return initialized;
	}

	// ------------------------------------------------------------------------
	bool Renderer::IsInitialized()
	{
		if (!this) return false;
		return initialized;
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

		UpdateJitter();

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
			SceneChanged();
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

		SceneChanged();
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
	void Renderer::SetCameraJitter(double jitterX, double jitterY)
	{
		this->jitterX = jitterX;
		this->jitterY = jitterY;

		UpdateJitter();
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
		if (view)
		{
			camera->SetZPlanes(view->GetVisibleBounds());
		}

		// Set up the camera (matrices)
		camera->Draw();
	}

	// ------------------------------------------------------------------------
	void Renderer::SceneChanged()
	{
		// Reset texture manager binding cache
		if (tm) tm->BeginNewPass();
		// Refocus camera
		if (camera && view) camera->focus = view->GetVisibleCenter();
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
	void Renderer::UpdateJitter()
	{
		// Set jitter values for subpixel rendering
		camera->jitterX = jitterX / static_cast<double>(viewportWidth) * 2.0;
		camera->jitterY = jitterY / static_cast<double>(viewportHeight) * 2.0;
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
		for (unsigned int objectId = 0; 
			objectId < view->GetNumberOfRenderables(); 
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
