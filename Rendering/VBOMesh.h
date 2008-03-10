#pragma once

#include "Renderable.h"

#include "GLBlaat/GLBuffer.h"
#include <cassert>

#define BUFFER_OFFSET(i) ((char *)0 + (i))

namespace NQVTK
{
	namespace AttributePointers
	{
		class AttributePointer
		{
		public:
			GLenum type;
			int stride;
			int offset;

			AttributePointer(GLenum type, int stride, int offset)
				: type(type), stride(stride), offset(offset) { }

			virtual void Set() const = 0;
		};

		class VertexPointer : public AttributePointer
		{
		public:
			int size;

			VertexPointer(int size, GLenum type, int stride, int offset)
				: AttributePointer(type, stride, offset), size(size) { }

			virtual void Set() const
			{
				glVertexPointer(size, type, stride, BUFFER_OFFSET(offset));
			}
		};

		class NormalPointer : public AttributePointer
		{
		public:
			NormalPointer(GLenum type, int stride, int offset)
				: AttributePointer(type, stride, offset) { }

			virtual void Set() const
			{
				glNormalPointer(type, stride, BUFFER_OFFSET(offset));
			}
		};

		class ColorPointer : public AttributePointer
		{
		public:
			int size;

			ColorPointer(int size, GLenum type, int stride, int offset)
				: AttributePointer(type, stride, offset), size(size) { }

			virtual void Set() const
			{
				glColorPointer(size, type, stride, BUFFER_OFFSET(offset));
			}
		};

		class TexCoordPointer : public AttributePointer
		{
		public:
			unsigned int index;
			int size;

			TexCoordPointer(unsigned int index, int size, 
				GLenum type, int stride, int offset)
				: AttributePointer(type, stride, offset), index(index), size(size) { }

			virtual void Set() const
			{
				glClientActiveTexture(GL_TEXTURE0 + index);
				glTexCoordPointer(size, type, stride, BUFFER_OFFSET(offset));
			}
		};

		class VertexAttribPointer : public AttributePointer
		{
		public:
			unsigned int index;
			int size;
			bool normalized;

			VertexAttribPointer(unsigned int index, int size, 
				GLenum type, bool normalized, int stride, int offset)
				: AttributePointer(type, stride, offset), index(index), 
				size(size), normalized(normalized) { }

			virtual void Set() const
			{
				glVertexAttribPointer(index, size, type, 
					(normalized ? GL_TRUE : GL_FALSE), 
					stride, BUFFER_OFFSET(offset));
			}
		};
	}

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
			for (std::vector<AttributePointers::AttributePointer*>::const_iterator it = 
				pointers.begin(); it != pointers.end(); ++it)
			{
				(*it)->Set();
			}
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
