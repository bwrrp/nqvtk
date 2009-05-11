#pragma once

#include "Renderer.h"

namespace NQVTK
{
	class SliceRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		SliceRenderer();
		virtual ~SliceRenderer();

		virtual void PrepareForRenderable(int objectId, 
			Renderable *renderable);

		virtual void Draw();
	};
}
