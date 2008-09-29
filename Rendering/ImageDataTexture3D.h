#pragma once

#include "GLBlaat/GLTexture3D.h"

#include "Math/Vector3.h"

#include <QObject>

#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>

#include <algorithm>

namespace NQVTK
{
	class ImageDataTexture3D : public GLTexture3D
	{
	public:
		typedef GLTexture3D Superclass;

		static ImageDataTexture3D *New(vtkImageData *data)
		{
			if (!data) return 0;

			qDebug("Converting imagedata...");

			int numComps = data->GetNumberOfScalarComponents();
			qDebug("Found %d components", numComps);

			// Get the scalar range of the image
			double range[2];
			data->GetScalarRange(range);
			// Adjust for additional components
			for (int i = 1; i < numComps; ++i)
			{
				double compRange[2];
				vtkDataArray *s = data->GetPointData()->GetScalars();
				s->GetRange(compRange, i);
				range[0] = std::min(range[0], compRange[0]);
				range[1] = std::max(range[1], compRange[1]);
			}
			qDebug("Range before conversion: %g..%g", range[0], range[1]);

			double shift = range[0];
			double scale = range[1] - range[0];
			qDebug("Shift: %g, scale: %g", shift, scale);

			// Convert to texture format
			vtkSmartPointer<vtkImageShiftScale> convert = 
				vtkSmartPointer<vtkImageShiftScale>::New();
			convert->SetInput(data);
			convert->SetShift(-shift);
			convert->SetScale(255.0 / scale);
			convert->SetOutputScalarTypeToUnsignedChar();
			convert->ClampOverflowOn();
			convert->Update();
			vtkImageData *vol = convert->GetOutput();

			// Check resulting range
			vol->GetScalarRange(range);
			for (int i = 0; i < numComps; ++i)
			{
				double compRange[2];
				vtkDataArray *s = vol->GetPointData()->GetScalars();
				s->GetRange(compRange, i);
				qDebug("Component %d range after conversion: %g..%g", 
					i, compRange[0] / 255.0, compRange[1] / 255.0);
				range[0] = std::min(range[0], compRange[0]);
				range[1] = std::max(range[1], compRange[1]);
			}
			qDebug("Combined range after conversion: %g..%g", 
				range[0] / 255.0, range[1] / 255.0);

			double bounds[6];
			data->GetBounds(bounds);
			Vector3 origin = Vector3(bounds[0], bounds[2], bounds[4]);
			Vector3 size = Vector3(
				bounds[1] - bounds[0], 
				bounds[3] - bounds[2], 
				bounds[5] - bounds[4]);
			qDebug("Origin: (%g, %g, %g), size: (%g, %g, %g)", 
				origin.x, origin.y, origin.z, 
				size.x, size.y, size.z);

			// Get image dimensions
			int dim[3];
			vol->GetDimensions(dim);
			int totalSize = dim[0] * dim[1] * dim[2];
			qDebug("Dimensions: %d x %d x %d", dim[0], dim[1], dim[2]);

			// TODO: check for NPOTS extension if needed

			// Determine GL formats to use
			int format = GL_NONE;

			switch (numComps)
			{
			case 1:
				format = GL_LUMINANCE;
				break;
			case 2:
				format = GL_LUMINANCE_ALPHA;
				break;
			case 3:
				format = GL_RGB;
				break;
			case 4:
				format = GL_RGBA;
				break;
			default:
				qDebug("Volume has unsupported number of components!");
				return 0;
			};

			// Finally, create the texture
			unsigned char *dataPointer = 
				static_cast<unsigned char*>(vol->GetScalarPointer());
			ImageDataTexture3D *tex = new ImageDataTexture3D(
				dim[0], dim[1], dim[2], format);
			tex->dataShift = shift;
			tex->dataScale = scale;
			tex->origin = origin;
			tex->originalSize = size;
			if (!tex->Allocate(format, 
				GL_UNSIGNED_BYTE, dataPointer)) 
			{
				delete tex;
				return 0;
			}
			return tex;
		}

		ImageDataTexture3D(int width, int height, int depth, int internalformat) 
			: GLTexture3D(width, height, depth, internalformat) { }

		float GetDataShift() const { return dataShift; }
		float GetDataScale() const { return dataScale; }
		Vector3 GetOriginalSize() const { return originalSize; }
		Vector3 GetOrigin() const { return origin; }

	private:
		double dataShift;
		double dataScale;
		Vector3 origin;
		Vector3 originalSize;
	};
}
