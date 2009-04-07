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

		virtual bool Initialize();

		virtual void Resize(int w, int h);

		virtual void Draw();

	protected:
		GLFramebuffer *shadowBuffer;
		NQVTK::Styles::ShadowMap *shadowStyle;
		NQVTK::LayeredRenderer *shadowRenderer;
	};
}
