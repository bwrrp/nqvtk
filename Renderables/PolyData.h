#pragma once

#include "VBOMesh.h"

#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkDataArray.h>

#define NQVTK_USE_NV_PRIMITIVE_RESTART

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

		PolyData(vtkPolyData *data)
		: vertIndices(0), lineIndices(0), polyIndices(0), stripIndices(0)
		{
			qDebug("Loading PolyData object...");

			// Store bounds
			data->GetBounds(bounds);
			qDebug("Bounds: %f - %f, %f - %f, %f - %f", 
				bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);

			// Get points
			numPoints = data->GetNumberOfPoints();
			qDebug("# points: %d", numPoints);
			vtkPoints *points = data->GetPoints();
			// TODO: can vtk points have #comps other than 3?
			AttributeSet *pointsSet = new NQVTK::AttributeSet(
				GLTypeFromVTKType(points->GetDataType()), 3);
			pointsSet->SetData(numPoints, points->GetVoidPointer(0));
			pointsSet->UseAsVertices();
			AddAttributeSet("gl_Vertex", pointsSet);

			// Do we have normals?
			// TODO: add support for cell normals if point normals are not available
			vtkDataArray *normals = data->GetPointData()->GetNormals();
			hasNormals = (normals != 0);
			qDebug("Has normals: %s", (hasNormals ? "yes" : "no"));
			if (hasNormals)
			{
				AttributeSet *normalsSet = new NQVTK::AttributeSet(
					GLTypeFromVTKType(normals->GetDataType()), 
					normals->GetNumberOfComponents());
				normalsSet->SetData(normals->GetNumberOfTuples(), 
					normals->GetVoidPointer(0));
				normalsSet->UseAsNormals();
				AddAttributeSet("gl_Normal", normalsSet);
			}

			// TODO: does vtk have colors?
			//vtkDataArray *colors = data->GetPointData()->GetS...

			// Do we have texture coordinates?
			vtkDataArray *tcoords = data->GetPointData()->GetTCoords();
			hasTCoords = (tcoords != 0);
			qDebug("Has tcoords: %s", (hasNormals ? "yes" : "no"));
			int tcoordsSize = 0;
			int tcoordsType = GL_NONE;
			if (hasTCoords)
			{
				AttributeSet *tcoordsSet = new NQVTK::AttributeSet(
					GLTypeFromVTKType(tcoords->GetDataType()), 
					tcoords->GetNumberOfComponents());
				tcoordsSet->SetData(tcoords->GetNumberOfTuples(), 
					tcoords->GetVoidPointer(0));
				tcoordsSet->UseAsTexCoords(0);
				AddAttributeSet("gl_TexCoord", tcoordsSet);
			}

			// Pointdata arrays can become custom attributes
			int numArrays = data->GetPointData()->GetNumberOfArrays();
			for (int arId = 0; arId < numArrays; ++arId)
			{
				vtkDataArray *ar = data->GetPointData()->GetArray(arId);
				qDebug("Found array '%s': %d tuples of %d type %d components", 
					ar->GetName(), ar->GetNumberOfTuples(), 
					ar->GetNumberOfComponents(), ar->GetDataType());

				// TODO: only load the data if this is a "wanted" array

				// Skip null names
				if (!ar->GetName()) continue;

				assert(ar->GetNumberOfTuples() == numPoints);

				AttributeSet *arSet = new AttributeSet(
					GLTypeFromVTKType(ar->GetDataType()), 
					ar->GetNumberOfComponents());
				arSet->SetData(ar->GetNumberOfTuples(), 
					ar->GetVoidPointer(0));
				AddAttributeSet(ar->GetName(), arSet, true);
			}

			// Primitive types in the polydata
			numVerts = data->GetNumberOfVerts();
			qDebug("# verts: %d", numVerts);
			numLines = data->GetNumberOfLines();
			qDebug("# lines: %d", numLines);
			numPolys = data->GetNumberOfPolys();
			qDebug("# polys: %d", numPolys);
			numStrips = data->GetNumberOfStrips();
			qDebug("# strips: %d", numStrips);
			// Not sure about this
			qDebug("# pieces: %d", data->GetNumberOfPieces());

			// Vertices
			if (numVerts > 0)
			{
				qDebug("Processing vertices...");
				vtkCellArray *verts = data->GetVerts();
				vtkIdType *pIds = verts->GetPointer();
				vtkIdType *pEnd = pIds + verts->GetNumberOfConnectivityEntries();
				vertIndices = GLBuffer::New();
				vertIndices->BindAsIndexData();
				vertIndices->SetData(
					verts->GetNumberOfConnectivityEntries() * sizeof(GLuint), 
					0, GL_STATIC_DRAW);
				unsigned int *indices = 
					reinterpret_cast<unsigned int*>(vertIndices->Map(GL_WRITE_ONLY));
				// Walk through vert cells and build the index buffer
				while (pIds < pEnd)
				{
					// Get number of points in cell
					vtkIdType nPts = *pIds;
					++pIds;
					// Copy indices
					while (nPts > 0)
					{
						*indices = static_cast<unsigned int>(*pIds);
						++indices;
						++pIds;
						--nPts;
					}
				}
				vertIndices->Unmap();
				vertIndices->Unbind();
			}

			// Lines
			if (numLines > 0)
			{
				qDebug("Processing lines...");
				// For now, we split up polylines into separate segments
				vtkCellArray *lines = data->GetLines();
				int numPolyLines = numLines;
				// Each line consists of number N followed by N point indices
				// This is translated to 2(N-1) points for line segments
				int numPoints = lines->GetNumberOfConnectivityEntries() - numPolyLines;
				numLines = numPoints - numPolyLines;
				vtkIdType *pIds = lines->GetPointer();
				vtkIdType *pEnd = pIds + lines->GetNumberOfConnectivityEntries();
				lineIndices = GLBuffer::New();
				lineIndices->BindAsIndexData();
				lineIndices->SetData(
					numLines * 2 * sizeof(GLuint), 
					0, GL_STATIC_DRAW);
				unsigned int *indices = 
					reinterpret_cast<unsigned int*>(lineIndices->Map(GL_WRITE_ONLY));
				// Walk through line cells and build the index buffer
				while (pIds < pEnd)
				{
					// Get number of points in polyline
					vtkIdType nPts = *pIds;
					++pIds;
					if (nPts == 1)
					{
						++pIds;
						continue;
					}
					// Split this polyline in segments
					while (nPts > 1)
					{
						// Add the first point
						*indices = static_cast<unsigned int>(*pIds);
						++indices;
						++pIds;
						--nPts;
						// Add the second point
						*indices = static_cast<unsigned int>(*pIds);
						++ indices;
					}
					++pIds;
				}
				lineIndices->Unmap();
				lineIndices->Unbind();
			}

			// Polygons
			if (numPolys > 0)
			{
				qDebug("Processing polygons...");
				vtkCellArray *polys = data->GetPolys();
				vtkIdType *pIds = polys->GetPointer();
				vtkIdType *pEnd = pIds + polys->GetNumberOfConnectivityEntries();
				polyIndices = GLBuffer::New();
				polyIndices->BindAsIndexData();
				polyIndices->SetData(
					polys->GetNumberOfCells() * 3 * sizeof(GLuint), 
					0, GL_STATIC_DRAW);
				unsigned int *indices = 
					reinterpret_cast<unsigned int*>(polyIndices->Map(GL_WRITE_ONLY));
				// Walk through poly cells and build the index buffer
				while (pIds < pEnd)
				{
					// Get number of points in cell
					vtkIdType nPts = *pIds;
					++pIds;
					// TODO: add support for quads and other polygons
					while (nPts > 3)
					{
						++pIds;
						--nPts;
					}
					// Copy indices
					while (nPts > 0)
					{
						*indices = static_cast<unsigned int>(*pIds);
						++indices;
						++pIds;
						--nPts;
					}
				}
				polyIndices->Unmap();
				polyIndices->Unbind();
			}

			// Strips
			if (numStrips > 0)
			{
				qDebug("Processing strips...");
				vtkCellArray *strips = data->GetStrips();
#ifdef NQVTK_USE_NV_PRIMITIVE_RESTART
				// Use the NV_primitive_restart extension to define an index to restart strips
				primitiveRestartIndex = 0xFFFF;
				// Each strip consists of number N followed by N point indices
				// Adding restart indices, we need N + 1 indices
				// We (ab)use numStrips to indicate the number of indices in the strip
				numStrips = strips->GetNumberOfConnectivityEntries();
#else
				// For now, we use degenerate triangles to combine all strips into one
				// Each strip consists of number N followed by N point indices
				// Adding degenerate tris adds two points per strip, so we have N + 2 points
				// We (ab)use numStrips to indicate the number of points in the strip
				numStrips = strips->GetNumberOfConnectivityEntries() + numStrips;
#endif
				vtkIdType *pIds = strips->GetPointer();
				vtkIdType *pEnd = pIds + strips->GetNumberOfConnectivityEntries();
				stripIndices = GLBuffer::New();
				stripIndices->BindAsIndexData();
				stripIndices->SetData(
					numStrips * sizeof(GLuint), 
					0, GL_STATIC_DRAW);
				unsigned int *indices = 
					reinterpret_cast<unsigned int*>(stripIndices->Map(GL_WRITE_ONLY));
				bool atStart = true;
				// Walk through strip cells and build the index buffer
				while (pIds < pEnd)
				{
#ifdef NQVTK_USE_NV_PRIMITIVE_RESTART
					// Start a new strip
					if (!atStart)
					{
						*indices = primitiveRestartIndex;
						++indices;
					}
#endif
					// Get number of points in strip
					vtkIdType nPts = *pIds;
					++pIds;
#ifndef NQVTK_USE_NV_PRIMITIVE_RESTART
					// Add the first point twice to finish the degenerate strip
					if (nPts > 0 && !atStart)
					{
						*indices = static_cast<unsigned int>(*pIds);
						++indices;
					}
#endif
					// Copy this strip
					while (nPts > 0)
					{
						*indices = static_cast<unsigned int>(*pIds);
						++indices;
#ifndef NQVTK_USE_NV_PRIMITIVE_RESTART
						// Add the last point twice to start a degenerate strip
						if (nPts == 1)
						{
							*indices = static_cast<unsigned int>(*pIds);
							++indices;
						}
#endif
						--nPts;
						++pIds;
					}
					atStart = false;
				}
				stripIndices->Unmap();
				stripIndices->Unbind();
			}
		}

		virtual ~PolyData() { }

		virtual void Draw() const
		{
			Draw(DRAWMODE_NORMAL, DRAWPARTS_ALL);
		}

		virtual void Draw(DrawMode mode, DrawParts parts) const
		{
			// Color
			glColor4d(color.x, color.y, color.z, opacity);

			// Enter object coordinates
			PushTransforms();

			// Setup vbo and pointers
			BindAttributes();

			// Use lighting if we have normal data
			if (hasNormals)
			{
				glEnable(GL_LIGHTING);
				glEnable(GL_LIGHT0);
				//glEnable(GL_NORMALIZE);
			}
			else
			{
				glDisable(GL_LIGHTING);
			}

			// Vertices
			if ((numVerts > 0) && (parts & DRAWPARTS_VERTS))
			{
				vertIndices->BindAsIndexData();
				glDrawElements(GL_POINTS, numVerts, GL_UNSIGNED_INT, 
					BUFFER_OFFSET(0));
				vertIndices->Unbind();
			}

			// Lines
			if ((numLines > 0) && (parts & DRAWPARTS_LINES))
			{
				lineIndices->BindAsIndexData();
				if (mode == DRAWMODE_POINTS)
				{
					glDrawElements(GL_POINTS, numLines * 2, 
						GL_UNSIGNED_INT, BUFFER_OFFSET(0));
				}
				else
				{
					glDrawElements(GL_LINES, numLines * 2, 
						GL_UNSIGNED_INT, BUFFER_OFFSET(0));
					// TODO: use glMultiDrawElements rather than duplicating points
				}
				lineIndices->Unbind();
			}

			// Polygons
			if ((numPolys > 0) && (parts & DRAWPARTS_POLYS))
			{
				polyIndices->BindAsIndexData();
				switch (mode)
				{
				case DRAWMODE_POINTS:
					glDrawElements(GL_POINTS, numPolys * 3, 
						GL_UNSIGNED_INT, BUFFER_OFFSET(0));
					break;
				case DRAWMODE_WIREFRAME:
					// TODO: wireframe mode
					break;
				case DRAWMODE_NORMAL:
					glDrawElements(GL_TRIANGLES, numPolys * 3, 
						GL_UNSIGNED_INT, BUFFER_OFFSET(0));
					break;
				}
				polyIndices->Unbind();
			}
			if ((numStrips > 0) && (parts & DRAWPARTS_STRIPS))
			{
				stripIndices->BindAsIndexData();
#ifdef NQVTK_USE_NV_PRIMITIVE_RESTART
				glPrimitiveRestartIndexNV(primitiveRestartIndex);
				glEnableClientState(GL_PRIMITIVE_RESTART_NV);
#endif
				switch (mode)
				{
				case DRAWMODE_POINTS:
					glDrawElements(GL_POINTS, numStrips, 
						GL_UNSIGNED_INT, BUFFER_OFFSET(0));
					break;
				case DRAWMODE_WIREFRAME:
					// TODO: draw as wireframe
					break;
				case DRAWMODE_NORMAL:
					glDrawElements(GL_TRIANGLE_STRIP, numStrips, 
						GL_UNSIGNED_INT, BUFFER_OFFSET(0));
					// TODO: use glMultiDrawElements rather than degenerate triangles?
					break;
				}
#ifdef NQVTK_USE_NV_PRIMITIVE_RESTART
				glDisableClientState(GL_PRIMITIVE_RESTART_NV);
#endif
				stripIndices->Unbind();
			}

			// Unset vbo render state
			UnbindAttributes();

			// Restore world coordinates
			PopTransforms();
		}

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

		GLenum GLTypeFromVTKType(int vtkType)
		{
			switch (vtkType)
			{
			case VTK_FLOAT:
				return GL_FLOAT;
			case VTK_DOUBLE:
				return GL_DOUBLE;
			default:
				qDebug("Error! Unsupported VTK data type: %d", vtkType);
				return GL_NONE;
			}
		}

	private:
		// Not implemented
		PolyData(const PolyData&);
		void operator=(const PolyData&);
	};
}