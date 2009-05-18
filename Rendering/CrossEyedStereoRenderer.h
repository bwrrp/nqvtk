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

		virtual bool Initialize();

		virtual void SetViewport(int x, int y, int w, int h);

		virtual void Clear();

		virtual void Draw();

		virtual void SetScene(Scene *scene);

		double eyeSpacing;

	protected:
		GLFramebuffer *fboEyeHelper;

	private:
		// Not implemented
		CrossEyedStereoRenderer(const CrossEyedStereoRenderer&);
		void operator=(const CrossEyedStereoRenderer&);
	};
}
