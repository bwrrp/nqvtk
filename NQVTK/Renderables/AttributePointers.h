#pragma once

#include "GLBlaat/GL.h"

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
			virtual void Unset() const = 0;
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
				glEnableClientState(GL_VERTEX_ARRAY);
			}

			virtual void Unset() const
			{
				glDisableClientState(GL_VERTEX_ARRAY);
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
				glEnableClientState(GL_NORMAL_ARRAY);
			}

			virtual void Unset() const
			{
				glDisableClientState(GL_NORMAL_ARRAY);
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
				glEnableClientState(GL_COLOR_ARRAY);
			}

			virtual void Unset() const
			{
				glDisableClientState(GL_COLOR_ARRAY);
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
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			}

			virtual void Unset() const
			{
				glClientActiveTexture(GL_TEXTURE0 + index);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
		};

		class VertexAttribPointer : public AttributePointer
		{
		public:
			int index;
			int size;
			bool normalized;

			VertexAttribPointer(int index, int size, 
				GLenum type, bool normalized, int stride, int offset)
				: AttributePointer(type, stride, offset), index(index), 
				size(size), normalized(normalized) { }

			virtual void Set() const
			{
				// Ignore placeholder pointers (index < 0)
				if (index >= 0)
				{
					glVertexAttribPointer(index, size, type, 
						(normalized ? GL_TRUE : GL_FALSE), 
						stride, BUFFER_OFFSET(offset));
					glEnableVertexAttribArray(index);
				}
			}

			virtual void Unset() const 
			{
				// Ignore placeholder pointers (index < 0)
				if (index >= 0)
				{
					glDisableVertexAttribArray(index);
				}
			}
		};
	}
}
