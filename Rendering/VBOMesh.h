#pragma once

#include "Renderable.h"

#include "AttributePointers.h"

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
			ClearPointers();
			delete vertexBuffer;
		}

		// Pointers
		void VertexPointer(int size, GLenum type, int stride, int offset)
		{
			pointers.push_back(new AttributePointers::VertexPointer(
				size, type, stride, offset));			
		}
		void NormalPointer(GLenum type, int stride, int offset)
		{
			pointers.push_back(new AttributePointers::NormalPointer(
				type, stride, offset));
		}
		void ColorPointer(int size, GLenum type, int stride, int offset)
		{
			pointers.push_back(new AttributePointers::ColorPointer(
				size, type, stride, offset));
		}
		void TexCoordPointer(unsigned int index, int size, 
			GLenum type, int stride, int offset)
		{
			pointers.push_back(new AttributePointers::TexCoordPointer(
				index, size, type, stride, offset));
		}
		void VertexAttribPointer(unsigned int index, int size, GLenum type, 
			bool normalized, int stride, int offset)
		{
			pointers.push_back(new AttributePointers::VertexAttribPointer(
				index, size, type, normalized, stride, offset));
		}

		void SetPointers() const
		{
			vertexBuffer->BindAsVertexData();
			for (std::vector<AttributePointers::AttributePointer*>::const_iterator it = 
				pointers.begin(); it != pointers.end(); ++it)
			{
				(*it)->Set();
			}
		}

		void UnsetPointers() const
		{
			for (std::vector<AttributePointers::AttributePointer*>::const_iterator it = 
				pointers.begin(); it != pointers.end(); ++it)
			{
				(*it)->Unset();
			}
			vertexBuffer->Unbind();
		}

		void ClearPointers()
		{
			for (std::vector<AttributePointers::AttributePointer*>::const_iterator it = 
				pointers.begin(); it != pointers.end(); ++it)
			{
				delete (*it);
			}
			pointers.clear();
		}

		virtual void Draw() const = 0;

	protected:
		GLBuffer *vertexBuffer;
		std::vector<AttributePointers::AttributePointer*> pointers;

	private:
		// Not implemented
		VBOMesh(const VBOMesh&);
		void operator=(const VBOMesh&);
	};
}
