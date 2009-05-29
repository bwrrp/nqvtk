#pragma once

#include "Renderer.h"

namespace NQVTK
{
	class NestedRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		NestedRenderer(Renderer *baseRenderer);
		virtual ~NestedRenderer();

		Renderer *GetBaseRenderer() { return baseRenderer; }

		virtual void SetViewport(int x, int y, int w, int h);

		virtual void Draw() = 0;

		virtual void DrawCamera();

		virtual void SceneChanged();

		virtual void SetView(NQVTK::View *view);

	protected:
		Renderer *baseRenderer;

		virtual void UpdateCamera();

	private:
		// Not implemented
		NestedRenderer(const NestedRenderer&);
		void operator=(const NestedRenderer&);
	};
}
