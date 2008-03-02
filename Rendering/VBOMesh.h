#pragma once

#include "Renderable.h"

#include "GLBlaat/GLBuffer.h"
#include <cassert>

namespace NQVTK
{
	class VBOMesh : public Renderable
	{
	public:
		typedef Renderable Superclass;

		VBOMesh()
		{
			vertexBuffer = GLBuffer::New();
			assert(vertexBuffer);
		}

		virtual ~VBOMesh() 
		{ 
			delete vertexBuffer;
		}

		virtual void Draw()
		{
			// TODO: render VBOs
		}

	protected:
		GLBuffer *vertexBuffer;
	};
}
