#pragma once

#include "VBOMesh.h"

#include <string>
#include <vtkPolyData.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>

#define BUFFER_OFFSET(i) ((char *)0 + (i))

namespace NQVTK
{
	class PolyData : public VBOMesh
	{
	public:
		typedef VBOMesh Superclass;

		PolyData(const std::string &filename)
		: vertIndices(0), lineIndices(0), polyIndices(0), stripIndices(0)
		{
			// TODO: define some useful VBOMesh api and refactor this

			// TODO: check: polydata datatype should be float

			// Load a polydata for testing
			vtkSmartPointer<vtkXMLPolyDataReader> reader = 
				vtkSmartPointer<vtkXMLPolyDataReader>::New();
			reader->SetFileName(filename.c_str());
			reader->Update();

			qDebug("Loaded PolyData...");

			vtkPolyData *data = reader->GetOutput();

			data->GetBounds(bounds);
			qDebug("Bounds: %f - %f, %f - %f, %f - %f", 
				bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);

			// Get points
			numPoints = data->GetNumberOfPoints();
			qDebug("# points: %d", numPoints);	//# points: 33221
			vtkPoints *points = data->GetPoints();

			// Do we have normals?
			// TODO: add support for cell normals if point normals are not available
			vtkDataArray *normals = data->GetPointData()->GetNormals();

			// TODO: add support for colors, texture coordinates, ...

			// Figure out how much space we need in the VBO
			int pointsSize = points->GetNumberOfPoints() * 3 * sizeof(GLfloat);
			int normalsSize = normals->GetNumberOfTuples() * 
				normals->GetNumberOfComponents() * sizeof(GLfloat);
			int totalSize = pointsSize + normalsSize;
			
			// Allocate VBO and copy data
			vertexBuffer->BindAsVertexData();
			vertexBuffer->SetData(totalSize, 0, GL_STATIC_DRAW);
			vertexBuffer->SetSubData(0, pointsSize, points->GetVoidPointer(0));
			glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
			hasNormals = (normals != 0);
			if (hasNormals)
			{
				vertexBuffer->SetSubData(pointsSize, normalsSize, normals->GetVoidPointer(0));
				glNormalPointer(GL_FLOAT, 0, BUFFER_OFFSET(pointsSize));
			}
			vertexBuffer->Unbind();

			// Primitive types in the polydata
			numVerts = data->GetNumberOfVerts();
			qDebug("# verts: %d", numVerts);	//# verts: 0
			numLines = data->GetNumberOfLines();
			qDebug("# lines: %d", numLines);	//# lines: 0
			numPolys = data->GetNumberOfPolys();
			qDebug("# polys: %d", numPolys);	//# polys: 66490
			numStrips = data->GetNumberOfStrips();
			qDebug("# strips: %d", numStrips);	//# strips: 0
			// Not sure about this
			qDebug("# pieces: %d", data->GetNumberOfPieces());	//# pieces: 1

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
				// TODO: draw data->GetLines() as lines
				numLines = 0;
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
				// TODO: draw data->GetStrips() as strips
				numStrips = 0;
			}
		}

		virtual ~PolyData() { }

		virtual void Draw()
		{
			// TODO: draw
			//Superclass::Draw();

			glColor3d(1.0, 1.0, 1.0);
			vertexBuffer->BindAsVertexData();
			// TODO: either VBOMesh or GLBuffer should manage these
			glEnableClientState(GL_VERTEX_ARRAY);
			if (hasNormals)
			{
				glEnable(GL_LIGHTING);
				glEnable(GL_LIGHT0);
				glEnable(GL_NORMALIZE);
				glEnableClientState(GL_NORMAL_ARRAY);
			}
			else
			{
				glDisable(GL_LIGHTING);
			}
			// TODO: render using the appropriate index buffers
			//glDrawArrays(GL_POINTS, 0, numPoints);
			if (numVerts > 0)
			{
				// Draw vertices
				vertIndices->BindAsIndexData();
				glDrawElements(GL_POINTS, numVerts, GL_UNSIGNED_INT, 
					BUFFER_OFFSET(0));
				vertIndices->Unbind();
			}
			if (numLines > 0)
			{
				// TODO: draw lines
				// Lines are multiple indexed polylines, 
				// glMultiDrawElements may work well here
			}
			if (numPolys > 0)
			{
				// Draw polys
				polyIndices->BindAsIndexData();
				glDrawElements(GL_TRIANGLES, numPolys * 3, GL_UNSIGNED_INT, 
					BUFFER_OFFSET((numVerts + numLines) * sizeof(GLuint)));
				polyIndices->Unbind();
			}
			if (numStrips > 0)
			{
				// TODO: draw triangle strips
				// Strips are multiple indexed triangle strips, 
				// glMultiDrawElements may work well here
			}
			vertexBuffer->Unbind();
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

		bool hasNormals;
	};
}
