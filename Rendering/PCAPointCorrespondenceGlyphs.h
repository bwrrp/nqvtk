#pragma once

#include "VBOMesh.h"

#include "GLBlaat/GLBuffer.h"

namespace NQVTK
{
	class PCAPointCorrespondenceGlyphs : public VBOMesh
	{
	public:
		typedef VBOMesh Superclass;

		PCAPointCorrespondenceGlyphs(VBOMesh *obj1, VBOMesh *obj2)
			: obj1(obj1), obj2(obj2)
		{
			// Extract points from the objects
			AttributeSet *set1 = obj1->GetAttributeSet("gl_Vertex");
			GLBuffer *vbo1 = set1->GetBuffer();
			AttributeSet *set2 = obj2->GetAttributeSet("gl_Vertex");
			GLBuffer *vbo2 = set2->GetBuffer();

			// TODO: add support for other data types
			assert(set1->GetValueType() == GL_DOUBLE);
			assert(set2->GetValueType() == GL_DOUBLE);

			assert(set1->GetNumberOfComponents() == 3);
			assert(set2->GetNumberOfComponents() == 3);

			// We can only map a single VBO at a time, so first copy to main memory
			int numPoints = set1->GetNumberOfTuples();
			assert(set2->GetNumberOfTuples() == numPoints);
			// Copy points from object 1
			std::vector<Vector3> points1;
			points1.resize(numPoints);
			double *data = static_cast<double*>(vbo1->Map(GL_READ_ONLY));
			for (int i = 0; i < numPoints; ++i)
			{
				points1[i] = Vector3 p(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
			}
			vbo1->Unmap();
			// TODO: also copy vbo2, and the eigenvectors!

			// TODO: create merged VBOs for points and eigenvectors
			
			// TODO: create index buffer for glyph lines
		}

		virtual void Draw() const
		{
			// TODO: draw glyphs
		}

	protected:
		VBOMesh *obj1;
		VBOMesh *obj2;
	};
}
