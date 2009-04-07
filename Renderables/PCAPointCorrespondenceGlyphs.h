#pragma once

#include "VBOMesh.h"

class GLBuffer;

namespace NQVTK
{
	class VBOMesh;

	class PCAPointCorrespondenceGlyphs : public VBOMesh
	{
	public:
		typedef VBOMesh Superclass;

		PCAPointCorrespondenceGlyphs(VBOMesh *obj1, VBOMesh *obj2);

		virtual void Draw() const;

	protected:
		int numLines;
		GLBuffer *lineIndices;
	};
}
