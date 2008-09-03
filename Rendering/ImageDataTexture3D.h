#pragma once

#include "GLBlaat/GLTexture3D.h"

#include "Math/Vector3.h"

#include <QObject>

#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
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

			// Get the scalar range of the image
			double range[2];
			data->GetScalarRange(range);
			qDebug("Range before conversion: %g..%g", range[0], range[1]);

			double shift = range[0];
			double scale = range[1] - range[0];
			qDebug("Shift: %g, scale: %g", shift, scale);

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

			vol->GetScalarRange(range);
			qDebug("Range after conversion: %g..%g", 
				(range[0] / 255.0), (range[1] / 255.0));

			// Get image dimensions
			int dim[3];
			vol->GetDimensions(dim);
			int totalSize = dim[0] * dim[1] * dim[2];
			qDebug("Dimensions: %d x %d x %d", dim[0], dim[1], dim[2]);
			
			// TODO: check for NPOTS extension if needed

			// Finally, create the texture
			unsigned char *dataPointer = static_cast<unsigned char*>(vol->GetScalarPointer());
			ImageDataTexture3D *tex = new ImageDataTexture3D(
				dim[0], dim[1], dim[2], GL_LUMINANCE);
			tex->dataShift = shift;
			tex->dataScale = scale;
			tex->origin = origin;
			tex->originalSize = size;
			if (!tex->Allocate(GL_LUMINANCE, GL_UNSIGNED_BYTE, dataPointer)) 
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
