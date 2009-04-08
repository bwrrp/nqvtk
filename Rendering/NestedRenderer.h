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

		virtual void Draw() = 0;

		virtual void DrawCamera();

	protected:
		Renderer *baseRenderer;

		virtual void UpdateCamera();

	private:
		// Not implemented
		NestedRenderer(const NestedRenderer&);
		void operator=(const NestedRenderer&);
	};
}
