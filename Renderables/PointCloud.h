#pragma once

#include "VBOMesh.h"

#include <vector>

class GLBuffer;

namespace NQVTK
{
	class PointCloud : public VBOMesh
	{
	public:
		typedef VBOMesh Superclass;

		// TODO: add a second constructor for creating point clouds from scratch
		PointCloud(VBOMesh *mesh, std::vector<unsigned int> &selection);
		~PointCloud();

		virtual void Draw() const;

	protected:
		GLBuffer *pointIndices;
		int numPoints;

	private:
		// Not implemented
		PointCloud(const PointCloud&);
		void operator=(const PointCloud&);
	};
}
