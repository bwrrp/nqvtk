#include "GLBlaat/GL.h"

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
	ImageDataVolume *ImageDataVolume::New(vtkImageData *data, 
		VolumeDataType type)
	{
		if (!data) return 0;

		std::cout << "Converting imagedata..." << std::endl;

		int numComps = data->GetNumberOfScalarComponents();
		std::cout << "Found " << numComps << " components" << std::endl;

		if (type == Auto)
		{
			switch (data->GetScalarType())
			{
			case VTK_FLOAT:
			case VTK_DOUBLE:
				std::cout << "Auto type: float" << std::endl;
				type = Float;
				break;

			default:
				std::cout << "Auto type: unsigned char" << std::endl;
				type = UnsignedChar;
			}
		}

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
		switch (type)
		{
		case UnsignedChar:
			convert->SetScale(255.0 / scale);
			convert->SetOutputScalarTypeToUnsignedChar();
			break;
		case Float:
			convert->SetScale(1.0 / scale);
			convert->SetOutputScalarTypeToFloat();
			break;
		default:
			std::cerr << "Unknown data type: " << type << std::endl;
			return 0;
		}
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
				" range after conversion: " << compRange[0] << 
				".." << compRange[1] << std::endl;
			range[0] = std::min(range[0], compRange[0]);
			range[1] = std::max(range[1], compRange[1]);
		}
		std::cout << "Combined range after conversion: " << 
			range[0] << ".." << range[1] << std::endl;

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
		int internalformat = GL_NONE;
		int dataformat = GL_NONE;
		int datatype = GL_NONE;
		if (numComps >  4 || numComps < 1)
		{
			std::cerr << "Volume has unsupported number of components!" << 
				std::endl;
			return 0;
		}
		else
		{
			switch (type)
			{
			case UnsignedChar:
				{
				int formats[] = {
					GL_LUMINANCE, 
					GL_LUMINANCE_ALPHA, 
					GL_RGB, 
					GL_RGBA};
				internalformat = formats[numComps - 1];
				dataformat = internalformat;
				datatype = GL_UNSIGNED_BYTE;
				}
				break;
			case Float:
				{
					// TODO: check for the relevant extensions...
					int internalformats[] = {
						GL_LUMINANCE16F_ARB, 
						GL_LUMINANCE_ALPHA16F_ARB, 
						GL_RGB16F, 
						GL_RGBA16F};
					internalformat = internalformats[numComps - 1];
					int dataformats[] = {
						GL_LUMINANCE, 
						GL_LUMINANCE_ALPHA, 
						GL_RGB, 
						GL_RGBA};
					dataformat = dataformats[numComps - 1];
					datatype = GL_FLOAT;
				}
				break;
			default:
				std::cerr << "Unknown GL data type: " << type << std::endl;
				return 0;
			}
		}

		// Finally, create the texture
		ImageDataVolume *tex = new ImageDataVolume(
			dim[0], dim[1], dim[2], internalformat);
		tex->dataShift = shift;
		tex->dataScale = scale;
		tex->origin = origin;
		tex->originalSize = size;
		if (!tex->Allocate(dataformat, datatype, vol->GetScalarPointer())) 
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
