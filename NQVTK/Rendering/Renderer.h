#pragma once

#include "NQVTK/Math/Vector3.h"

#include <vector>

class GLFramebuffer;
class GLTexture;
class GLTextureManager;

namespace NQVTK
{
	class Camera;
	class Renderable;
	class Scene;
	class View;

	class Renderer
	{
	public:
		Renderer();
		virtual ~Renderer();

		bool TryInitialize();
		virtual bool IsInitialized();

		void Move(int x, int y);
		void Resize(int w, int h);
		virtual void SetViewport(int x, int y, int w, int h);

		virtual void Clear();

		// To be implemented in derived classes
		virtual void Draw() = 0;

		virtual void DrawCamera();

		virtual void SceneChanged();

		void SetScene(Scene *scene);

		View *GetView() { return this->view; }
		virtual void SetView(View *view);

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

		bool initialized;

		Camera *camera;
		View *view;

		NQVTK::Vector3 lightPos;

		GLTextureManager *tm;

		GLFramebuffer *fboTarget;

		virtual bool Initialize();

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
