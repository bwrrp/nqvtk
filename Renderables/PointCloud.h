#pragma once

#include "VBOMesh.h"

#include "GLBlaat/GLBuffer.h"

#include <vector>

namespace NQVTK
{
	class PointCloud : public VBOMesh
	{
	public:
		typedef VBOMesh Superclass;

		PointCloud(VBOMesh *mesh, std::vector<unsigned int> &selection)
		{
			assert(mesh);

			// Copy the bounds of the original mesh
			mesh->GetBounds(bounds);

			// We share the mesh's points attribset
			// TODO: this is messy, what if the mesh is deleted?
			AttributeSet *pointsSet = mesh->GetAttributeSet("gl_Vertex");
			assert(pointsSet);
			AddAttributeSet("gl_Vertex", pointsSet);

			numPoints = selection.size();

			if (numPoints > 0)
			{
				// Create the index buffer
				pointIndices = GLBuffer::New();
				assert(pointIndices);
				pointIndices->BindAsIndexData();
				void *data = 0;
				if (selection.size() > 0) data = &selection[0];
				pointIndices->SetData(selection.size() * sizeof(GLuint), 
					data, GL_STATIC_DRAW);
				pointIndices->Unbind();
			}
			else
			{
				pointIndices = 0;
			}
		}

		// TODO: add a second constructor for creating point clouds from scratch

		~PointCloud()
		{
			// Remove the points attributeset, because it's shared
			attributeSets.erase("gl_Vertex");
			if (pointIndices) delete pointIndices;
		}

		virtual void Draw() const
		{
			// Color
			glColor4d(color.x, color.y, color.z, opacity);

			// Enter object coordinates
			PushTransforms();

			// Setup vbo and pointers
			BindAttributes();

			glPushAttrib(GL_ALL_ATTRIB_BITS);

			glDisable(GL_LIGHTING);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glEnable(GL_BLEND);

			if (numPoints > 0)
			{
				pointIndices->BindAsIndexData();
				glDrawElements(GL_POINTS, numPoints, 
					GL_UNSIGNED_INT, BUFFER_OFFSET(0));
				pointIndices->Unbind();
			}

			glPopAttrib();

			// Unset vbo render state
			UnbindAttributes();

			// Restore world coordinates
			PopTransforms();
		}

	protected:
		GLBuffer *pointIndices;
		int numPoints;

	private:
		// Not implemented
		PointCloud(const PointCloud&);
		void operator=(const PointCloud&);
	};
}
