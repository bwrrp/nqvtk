#pragma once

class GLBuffer;

namespace NQVTK
{
	namespace AttributePointers { class AttributePointer; }

	class AttributeSet
	{
	public:
		AttributeSet(unsigned int valueType, int numComponents);
		virtual ~AttributeSet();

		// Set the buffer data and usage hint
		// (default usage is GL_STATIC_DRAW)
		void SetData(int numTuples, void *data, unsigned int usage = 0x88E4);

		void Bind();
		void Unbind();

		void UseAsVertices();
		void UseAsNormals();
		void UseAsColors();
		void UseAsTexCoords(int index);
		void UseAsVertexAttrib(int index, bool normalized = false);
		void DontUse();

		int GetNumberOfTuples() { return numTuples; }
		int GetNumberOfComponents() { return numComponents; }
		unsigned int GetValueType() { return valueType; }

		// Advanced use only...
		GLBuffer *GetBuffer() { return buffer; }

	protected:
		GLBuffer *buffer;
		AttributePointers::AttributePointer *pointer;

		unsigned int valueType;
		int numComponents;
		int numTuples;

		int GetValueSize();

	private:
		// Not implemented
		AttributeSet(const AttributeSet&);
		void operator=(const AttributeSet&);
	};
}
