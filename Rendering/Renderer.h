#pragma once

#include "Math/Vector3.h"

#include <vector>

class GLFramebuffer;
class GLTexture;
class GLTextureManager;

namespace NQVTK
{
	class Camera;
	class Renderable;
	class Scene;

	class Renderer
	{
	public:
		Renderer();
		virtual ~Renderer();

		virtual bool Initialize();

		void Move(int x, int y);
		void Resize(int w, int h);
		virtual void SetViewport(int x, int y, int w, int h);

		virtual void Clear();

		// To be implemented in derived classes
		virtual void Draw() = 0;

		virtual void DrawCamera();

		Scene *GetScene() { return this->scene; }
		virtual void SetScene(Scene *scene);

		Camera *GetCamera();
		void SetCamera(Camera *cam);

		GLFramebuffer *SetTarget(GLFramebuffer *target);
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
		Scene *scene;
		NQVTK::Vector3 lightPos;

		GLTextureManager *tm;

		GLFramebuffer *fboTarget;

		virtual void UpdateLighting();

		// Hook for per-renderable processing
		virtual void PrepareForRenderable(int objectId, 
			Renderable *renderable);

		// Loop over and draw all renderables
		virtual void DrawRenderables();

		void DrawTexture(GLTexture *tex, 
			double xmin = -1.0, double xmax = 1.0, 
			double ymin = -1.0, double ymax = 1.0);

	private:
		// Not implemented
		Renderer(const Renderer&);
		void operator=(const Renderer&);
	};
}
