#pragma once

#include "VBOMesh.h"

#define NQVTK_USE_NV_PRIMITIVE_RESTART

class vtkPolyData;
class GLBuffer;

namespace NQVTK
{
	class PolyData : public VBOMesh
	{
	public:
		typedef VBOMesh Superclass;

		enum DrawMode
		{
			DRAWMODE_POINTS,
			DRAWMODE_WIREFRAME,
			DRAWMODE_NORMAL
		};

		enum DrawParts
		{
			DRAWPARTS_NONE = 0,
			DRAWPARTS_VERTS = 1,
			DRAWPARTS_LINES = 2,
			DRAWPARTS_POLYS = 4,
			DRAWPARTS_STRIPS = 8,
			DRAWPARTS_ALL = 15
		};

		PolyData(vtkPolyData *data);
		virtual ~PolyData();

		virtual void Draw() const;
		virtual void Draw(DrawMode mode, DrawParts parts) const;

	protected:
		int numPoints;

		int numVerts;
		int numLines;
		int numPolys;
		int numStrips;

		GLBuffer *vertIndices;
		GLBuffer *lineIndices;
		GLBuffer *polyIndices;
		GLBuffer *stripIndices;

#ifdef NQVTK_USE_NV_PRIMITIVE_RESTART
		unsigned int primitiveRestartIndex;
#endif

		bool hasNormals;
		bool hasTCoords;

	private:
		// Not implemented
		PolyData(const PolyData&);
		void operator=(const PolyData&);
	};
}
