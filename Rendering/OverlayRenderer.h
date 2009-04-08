#pragma once

#include "Renderer.h"

class GLTexture;
class GLFramebuffer;

namespace NQVTK
{
	class OverlayRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		OverlayRenderer(Renderer *base, Renderer *overlay);
		virtual ~OverlayRenderer();

		virtual bool Initialize();

		virtual void Resize(int w, int h);

		void DrawTexture(GLTexture *tex);

		virtual void Draw();

		Renderer *GetBaseRenderer() { return baseRenderer; }
		Renderer *GetOverlayRenderer() { return overlayRenderer; }

		GLTexture *GetBaseImage();
		GLTexture *GetOverlayImage();

		bool updateBase;
		bool updateOverlay;

	protected:
		Renderer *baseRenderer;
		Renderer *overlayRenderer;

		GLFramebuffer *baseFbo;
		GLFramebuffer *overlayFbo;

	private:
		// Not implemented
		OverlayRenderer(const OverlayRenderer&);
		void operator=(const OverlayRenderer&);
	};
}
