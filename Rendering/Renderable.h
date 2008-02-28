#pragma once

#include "GLBlaat/GLResource.h"

namespace NQVTK 
{
	class Renderable : public GLResource
	{
	public:
		typedef GLResource Superclass;

		Renderable() { };

		virtual ~Renderable() { }

		virtual void Draw() = 0;

		// TODO: Transformations?
		// TODO: Something about size, bounding box, ... (useful stuff)
	};
}
