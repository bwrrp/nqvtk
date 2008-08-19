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

		PointCloud(Renderable *mesh, std::vector<int> &selection)
		{
			assert(mesh);
			// NOTE: we can't simply copy the selected points, this would mean we 
			// need to figure out where the points are in the original mesh...

			// We share the mesh's VBO
			// TODO: this is messy, what if the mesh is deleted?
			sharedVBO = 0;//mesh->vertexBuffer;

			// TODO: find the mesh's vertex pointer and copy it
			//pointers = mesh->pointers;

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

		// TODO: add a second constructor for creating point clouds from scratch

		~PointCloud()
		{
			if (sharedVBO)
			{
				delete sharedVBO;
				pointers.clear();
			}
			if (pointIndices) delete pointIndices;
		}

		virtual void Draw() const
		{
			// Color
			glColor4d(color.x, color.y, color.z, opacity);

			// Enter object coordinates
			PushTransforms();

			// Setup vbo and pointers
			SetPointers();

			glDisable(GL_LIGHTING);

			if (numPoints > 0)
			{
				pointIndices->BindAsIndexData();
				glDrawElements(GL_POINTS, numPoints, GL_UNSIGNED_INT, 
					BUFFER_OFFSET(0));
				pointIndices->Unbind();
			}

			// Unset vbo render state
			UnsetPointers();

			// Restore world coordinates
			PopTransforms();
		}

	protected:
		GLBuffer *sharedVBO;
		GLBuffer *pointIndices;
		int numPoints;

	private:
		// Not implemented
		PointCloud(const PointCloud&);
		void operator=(const PointCloud&);
	};
}
