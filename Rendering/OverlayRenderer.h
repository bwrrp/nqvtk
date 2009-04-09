#pragma once

#include "NestedRenderer.h"

class GLFramebuffer;

namespace NQVTK
{
	class OverlayRenderer : public NestedRenderer
	{
	public:
		typedef Renderer Superclass;

		OverlayRenderer(Renderer *base, Renderer *overlay);
		virtual ~OverlayRenderer();

		virtual bool Initialize();

		virtual void SetViewport(int x, int y, int w, int h);

		virtual void Draw();

		// GetBaseRenderer is provided by NestedRenderer
		Renderer *GetOverlayRenderer() { return overlayRenderer; }

		GLTexture *GetBaseImage();
		GLTexture *GetOverlayImage();

		bool updateBase;
		bool updateOverlay;

	protected:
		Renderer *overlayRenderer;

		GLFramebuffer *baseFbo;
		GLFramebuffer *overlayFbo;

	private:
		// Not implemented
		OverlayRenderer(const OverlayRenderer&);
		void operator=(const OverlayRenderer&);
	};
}
