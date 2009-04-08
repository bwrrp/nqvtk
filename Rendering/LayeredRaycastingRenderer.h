#pragma once

#include "LayeredRenderer.h"

class GLFramebuffer;
class GLProgram;

namespace NQVTK
{
	class LayeredRaycastingRenderer : public NQVTK::LayeredRenderer
	{
	public:
		typedef NQVTK::LayeredRenderer Superclass;

		LayeredRaycastingRenderer();
		virtual ~LayeredRaycastingRenderer();

		virtual bool Initialize();

		virtual void Resize(int w, int h);

		virtual void DrawScribePass(int layer);

		virtual void DrawRaycastPass(int layer);

	protected:
		GLFramebuffer *fboG;
		GLProgram *raycaster;

		// Whether the current style requires layered raycasting
		bool layeredRaycasting;
	};
}
