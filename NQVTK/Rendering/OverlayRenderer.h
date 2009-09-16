#pragma once

#include "NestedRenderer.h"

class GLFramebuffer;

namespace NQVTK
{
	class OverlayRenderer : public NestedRenderer
	{
	public:
		typedef NestedRenderer Superclass;

		OverlayRenderer(Renderer *base, Renderer *overlay);
		virtual ~OverlayRenderer();

		virtual bool IsInitialized();

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

		virtual bool Initialize();

	private:
		// Not implemented
		OverlayRenderer(const OverlayRenderer&);
		void operator=(const OverlayRenderer&);
	};
}
