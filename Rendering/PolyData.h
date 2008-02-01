#pragma once

#include "Renderable.h"

#include <vtkPolyData.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkSmartPointer.h>

namespace NQVTK
{
	class PolyData : public Renderable
	{
	public:
		PolyData()
		{
			// Load a polydata for testing
			vtkSmartPointer<vtkXMLPolyDataReader> reader = 
				vtkSmartPointer<vtkXMLPolyDataReader>::New();
			reader->SetFileName(
				"D:/Data/msdata/T2W/T2W_images_normalized/T2W_normalized_GM/Gwn0200-TP_2004_07_08-T2.vtp");
			reader->Update();

			qDebug("Loaded PolyData...");

			vtkPolyData *data = reader->GetOutput();
			// TODO: extract geometry into VBOs
			// Some class would be useful to handle VBOs with vertex, normal, color, ... data
			
			// VTK points and cells
			qDebug("# points: %d", data->GetNumberOfPoints());
			qDebug("# cells: %d", data->GetNumberOfCells());

			// Primitive types in the polydata
			qDebug("# verts: %d", data->GetNumberOfVerts());	// to be rendered as points
			qDebug("# lines: %d", data->GetNumberOfLines());	// to be rendered as lines
			qDebug("# polys: %d", data->GetNumberOfPolys());	// to be rendered as polygons
			qDebug("# strips: %d", data->GetNumberOfStrips());	// to be rendered as triangle strips

			// Not sure about this
			qDebug("# pieces: %d", data->GetNumberOfPieces());
		}

		virtual ~PolyData() { }

		virtual void Draw()
		{
			// TODO: render VBOs
			glBegin(GL_TRIANGLES);
			glColor3d(1.0, 0.0, 0.0);
			glVertex3d(-1.0, -1.0, 0.0);
			glColor3d(0.0, 1.0, 0.0);
			glVertex3d(1.0, -1.0, 0.0);
			glColor3d(0.0, 0.0, 1.0);
			glVertex3d(0.0, 1.0, 0.0);
			glEnd();
		}
	};
}
