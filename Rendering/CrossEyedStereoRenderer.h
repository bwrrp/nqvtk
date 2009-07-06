#pragma once

#include "NestedRenderer.h"

class GLFramebuffer;

namespace NQVTK
{
	class CrossEyedStereoRenderer : public NestedRenderer
	{
	public:
		typedef Renderer Superclass;

		CrossEyedStereoRenderer(Renderer *baseRenderer);
		virtual ~CrossEyedStereoRenderer();

		virtual void SetViewport(int x, int y, int w, int h);

		virtual void Clear();

		virtual void Draw();

		double eyeSpacing;

	protected:
		GLFramebuffer *fboEyeHelper;

		virtual bool Initialize();

	private:
		// Not implemented
		CrossEyedStereoRenderer(const CrossEyedStereoRenderer&);
		void operator=(const CrossEyedStereoRenderer&);
	};
}
