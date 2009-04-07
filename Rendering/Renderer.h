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

	class Renderer
	{
	public:
		Renderer();
		virtual ~Renderer();

		virtual bool Initialize();

		virtual void Resize(int w, int h);

		virtual void Clear();

		virtual void DrawCamera();

		virtual void UpdateLighting();

		// To be implemented in derived classes
		virtual void Draw() = 0;

		// Hook for per-renderable processing
		virtual void PrepareForRenderable(int objectId, 
			Renderable *renderable);

		// Loop over and draw all renderables
		virtual void DrawRenderables();

		int AddRenderable(Renderable *obj);
		Renderable *GetRenderable(unsigned int i);
		Renderable *SetRenderable(unsigned int i, Renderable *obj);
		
		int GetNumberOfRenderables();
		
		void DeleteAllRenderables();

		// Replace the renderables with the given set
		// Beware of memory leaks!
		void SetRenderables(std::vector<Renderable*> renderables);

		void ResetRenderables();

		Camera *GetCamera();
		void SetCamera(Camera *cam);

		void ShareTextures(GLTextureManager *tm);

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
		std::vector<Renderable*> renderables;
		NQVTK::Vector3 lightPos;

		GLTextureManager *tm;

		GLFramebuffer *fboTarget;

		void TestDrawTexture(GLTexture *tex, 
			double xmin, double xmax, 
			double ymin, double ymax);

	private:
		// Not implemented
		Renderer(const Renderer&);
		void operator=(const Renderer&);
	};
}
