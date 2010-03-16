#include "AttributeSet.h"

#include "AttributePointers.h"

#include "GLBlaat/GLBuffer.h"

#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	AttributeSet::AttributeSet(GLenum valueType, int numComponents)
		: valueType(valueType), numComponents(numComponents), 
		buffer(GLBuffer::New()), pointer(0), 
		bufferShared(false), stride(0), offset(0)
	{
		assert(valueType != GL_NONE);
	}

	// ------------------------------------------------------------------------
	AttributeSet::AttributeSet(GLenum valueType, int numComponents, 
		GLBuffer *sharedBuffer, int stride, int offset)
		: valueType(valueType), numComponents(numComponents), 
		buffer(sharedBuffer), pointer(0), 
		bufferShared(true), stride(stride), offset(offset)
	{
		assert(valueType != GL_NONE);
	}

	// ------------------------------------------------------------------------
	AttributeSet::~AttributeSet()
	{
		if (!bufferShared) delete buffer;
		delete pointer;
	}

	// ------------------------------------------------------------------------
	void AttributeSet::SetData(int numTuples, void *data, GLenum usage)
	{
		assert(!bufferShared);

		buffer->BindAsVertexData();
		this->numTuples = numTuples;
		int size = numTuples * numComponents * GetValueSize();
		buffer->SetData(size, data, usage);
		buffer->Unbind();
	}

	// ------------------------------------------------------------------------
	void AttributeSet::Bind()
	{
		if (pointer)
		{
			buffer->BindAsVertexData();
			pointer->Set();
		}
	}

	// ------------------------------------------------------------------------
	void AttributeSet::Unbind()
	{
		if (pointer)
		{
			pointer->Unset();
			buffer->Unbind();
		}
	}

	// ------------------------------------------------------------------------
	void AttributeSet::UseAsVertices()
	{
		if (pointer) delete pointer;
		pointer = new NQVTK::AttributePointers::VertexPointer(
			numComponents, valueType, stride, offset);
	}

	// ------------------------------------------------------------------------
	void AttributeSet::UseAsNormals()
	{
		if (pointer) delete pointer;
		pointer = new NQVTK::AttributePointers::NormalPointer(
			valueType, stride, offset);
	}

	// ------------------------------------------------------------------------
	void AttributeSet::UseAsColors()
	{
		if (pointer) delete pointer;
		pointer = new NQVTK::AttributePointers::ColorPointer(
			numComponents, valueType, stride, offset);
	}

	// ------------------------------------------------------------------------
	void AttributeSet::UseAsTexCoords(int index)
	{
		if (pointer) delete pointer;
		pointer = new NQVTK::AttributePointers::TexCoordPointer(
			index, numComponents, valueType, stride, offset);
	}

	// ------------------------------------------------------------------------
	void AttributeSet::UseAsVertexAttrib(int index, bool normalized)
	{
		if (pointer) delete pointer;
		pointer = new NQVTK::AttributePointers::VertexAttribPointer(
			index, numComponents, valueType, normalized, stride, offset);
	}

	// ------------------------------------------------------------------------
	void AttributeSet::DontUse()
	{
		if (pointer) delete pointer;
		pointer = 0;
	}

	// ------------------------------------------------------------------------
	int AttributeSet::GetValueSize()
	{
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
}
