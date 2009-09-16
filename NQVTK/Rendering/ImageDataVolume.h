#pragma once

#include "Volume.h"

class vtkImageData;

namespace NQVTK
{
	class ImageDataVolume : public Volume
	{
	public:
		typedef Volume Superclass;

		enum VolumeDataType
		{
			UnsignedChar, 
			Float
		};

		static ImageDataVolume *New(vtkImageData *data, 
			VolumeDataType type = UnsignedChar);

	protected:
		ImageDataVolume(int width, int height, int depth, 
			int internalformat);
	};
}
