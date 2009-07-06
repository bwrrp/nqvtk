#include "AttributeSet.h"

#include "AttributePointers.h"

#include "GLBlaat/GLBuffer.h"

#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	AttributeSet::AttributeSet(GLenum valueType, int numComponents)
	{
		assert(valueType != GL_NONE);
		this->valueType = valueType;
		this->numComponents = numComponents;
		buffer = GLBuffer::New();
		// TODO: pointer
		pointer = 0;
	}

	// ------------------------------------------------------------------------
	AttributeSet::~AttributeSet()
	{
		delete buffer;
		delete pointer;
	}

	// ------------------------------------------------------------------------
	void AttributeSet::SetData(int numTuples, void *data, GLenum usage)
	{
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
			numComponents, valueType, 0, 0);
	}

	// ------------------------------------------------------------------------
	void AttributeSet::UseAsNormals()
	{
		if (pointer) delete pointer;
		pointer = new NQVTK::AttributePointers::NormalPointer(
			valueType, 0, 0);
	}

	// ------------------------------------------------------------------------
	void AttributeSet::UseAsColors()
	{
		if (pointer) delete pointer;
		pointer = new NQVTK::AttributePointers::ColorPointer(
			numComponents, valueType, 0, 0);
	}

	// ------------------------------------------------------------------------
	void AttributeSet::UseAsTexCoords(int index)
	{
		if (pointer) delete pointer;
		pointer = new NQVTK::AttributePointers::TexCoordPointer(
			index, numComponents, valueType, 0, 0);
	}

	// ------------------------------------------------------------------------
	void AttributeSet::UseAsVertexAttrib(int index, bool normalized)
	{
		if (pointer) delete pointer;
		pointer = new NQVTK::AttributePointers::VertexAttribPointer(
			index, numComponents, valueType, normalized, 0, 0);
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
}
