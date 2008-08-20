#pragma once

#include "GLBlaat/GLBuffer.h"

#include "AttributePointers.h"

namespace NQVTK
{
	class AttributeSet
	{
	public:
		AttributeSet(GLenum valueType, int numComponents)
		{
			assert(valueType != GL_NONE);
			this->valueType = valueType;
			this->numComponents = numComponents;
			buffer = GLBuffer::New();
			// TODO: pointer
			pointer = 0;
		}

		virtual ~AttributeSet()
		{
			delete buffer;
			if (pointer) delete pointer;
		}

		void SetData(int numTuples, void *data, GLenum usage = GL_STATIC_DRAW)
		{
			buffer->BindAsVertexData();
			this->numTuples = numTuples;
			int size = numTuples * numComponents * GetValueSize();
			buffer->SetData(size, data, usage);
			buffer->Unbind();
		}

		void Bind()
		{
			if (pointer)
			{
				buffer->BindAsVertexData();
				pointer->Set();
			}
		}

		void Unbind()
		{
			if (pointer)
			{
				pointer->Unset();
				buffer->Unbind();
			}
		}

		void UseAsVertices()
		{
			if (pointer) delete pointer;
			pointer = new NQVTK::AttributePointers::VertexPointer(
				numComponents, valueType, 0, 0);
		}

		void UseAsNormals()
		{
			if (pointer) delete pointer;
			pointer = new NQVTK::AttributePointers::NormalPointer(
				valueType, 0, 0);
		}

		void UseAsColors()
		{
			if (pointer) delete pointer;
			pointer = new NQVTK::AttributePointers::ColorPointer(
				numComponents, valueType, 0, 0);
		}

		void UseAsTexCoords(int index)
		{
			if (pointer) delete pointer;
			pointer = new NQVTK::AttributePointers::TexCoordPointer(
				index, numComponents, valueType, 0, 0);
		}

		void UseAsVertexAttrib(int index, bool normalized = false)
		{
			if (pointer) delete pointer;
			pointer = new NQVTK::AttributePointers::VertexAttribPointer(
				index, numComponents, valueType, normalized, 0, 0);
		}

		void DontUse()
		{
			if (pointer) delete pointer;
			pointer = 0;
		}

		int GetNumberOfTuples() { return numTuples; }
		int GetNumberOfComponents() { return numComponents; }

	protected:
		GLBuffer *buffer;
		AttributePointers::AttributePointer *pointer;

		GLenum valueType;
		int numComponents;
		int numTuples;

		int GetValueSize()
		{
			// NOTE: templates are not useful here, we still need the enum for OpenGL
			switch (valueType)
			{
			case GL_FLOAT:
				return sizeof(GLfloat);
			case GL_DOUBLE:
				return sizeof(GLdouble);
			default:
				return 0;
			}
		}

	private:
		// Not implemented
		AttributeSet(const AttributeSet&);
		void operator=(const AttributeSet&);
	};
}
