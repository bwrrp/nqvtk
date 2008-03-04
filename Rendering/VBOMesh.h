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

		virtual void Draw() const = 0;

	protected:
		GLBuffer *vertexBuffer;

	private:
		// Not implemented
		VBOMesh(const VBOMesh&);
		void operator=(const VBOMesh&);
	};
}
