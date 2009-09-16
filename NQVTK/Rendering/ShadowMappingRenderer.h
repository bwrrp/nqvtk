#pragma once

#include "LayeredRenderer.h"

class GLFrameBuffer;

namespace NQVTK
{
	namespace Styles { class ShadowMap; }

	class ShadowMappingRenderer : public LayeredRenderer
	{
	public:
		typedef LayeredRenderer Superclass;

		ShadowMappingRenderer();
		virtual ~ShadowMappingRenderer();

		virtual bool IsInitialized();

		virtual void SetViewport(int x, int y, int w, int h);

		virtual void Draw();

		virtual void SetView(View *view);

	protected:
		GLFramebuffer *shadowBuffer;
		NQVTK::Styles::ShadowMap *shadowStyle;
		NQVTK::LayeredRenderer *shadowRenderer;

		virtual bool Initialize();
	};
}
