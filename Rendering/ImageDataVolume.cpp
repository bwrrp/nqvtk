#include "ImageDataVolume.h"

#include "Math/Vector3.h"

#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>

#include <algorithm>
#include <iostream>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	ImageDataVolume *ImageDataVolume::New(vtkImageData *data)
	{
		if (!data) return 0;

		std::cout << "Converting imagedata..." << std::endl;

		int numComps = data->GetNumberOfScalarComponents();
		std::cout << "Found " << numComps << " components" << std::endl;

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
		std::cout << "Range before conversion: " <<
			range[0] << ".." << range[1] << std::endl;

		double shift = range[0];
		double scale = range[1] - range[0];
		std::cout << "Shift: " << shift <<
			", scale: " << scale << std::endl;

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
			std::cout << "Component " << i << 
				" range after conversion: " << compRange[0] / 255.0 << 
				".." << compRange[1] / 255.0 << std::endl;
			range[0] = std::min(range[0], compRange[0]);
			range[1] = std::max(range[1], compRange[1]);
		}
		std::cout << "Combined range after conversion: " << 
			range[0] / 255.0 << ".." << range[1] / 255.0 << std::endl;

		double bounds[6];
		data->GetBounds(bounds);
		Vector3 origin = Vector3(bounds[0], bounds[2], bounds[4]);
		Vector3 size = Vector3(
			bounds[1] - bounds[0], 
			bounds[3] - bounds[2], 
			bounds[5] - bounds[4]);
		std::cout << "Origin: (" << //%g, %g, %g), size: (%g, %g, %g)", 
			origin.x << ", " << origin.y << ", " << origin.z << "), size: (" <<
			size.x << ", " << size.y << ", " << size.z << ")" << std::endl;

		// Get image dimensions
		int dim[3];
		vol->GetDimensions(dim);
		int totalSize = dim[0] * dim[1] * dim[2];
		std::cout << "Dimensions: " << 
			dim[0] << " x " << 
			dim[1] << " x " << 
			dim[2] << std::endl;

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
			std::cerr << "Volume has unsupported number of components!" << std::endl;
			return 0;
		};

		// Finally, create the texture
		unsigned char *dataPointer = 
			static_cast<unsigned char*>(vol->GetScalarPointer());
		ImageDataVolume *tex = new ImageDataVolume(
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

	// ------------------------------------------------------------------------
	ImageDataVolume::ImageDataVolume(int width, int height, int depth, 
		int internalformat) 
		: Volume(width, height, depth, internalformat) { }
}
