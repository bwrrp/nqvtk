#pragma once

#include "GLBlaat/GLTexture3D.h"

#include "Math/Vector3.h"

class vtkImageData;

namespace NQVTK
{
	class ImageDataTexture3D : public GLTexture3D
	{
	public:
		typedef GLTexture3D Superclass;

		static ImageDataTexture3D *New(vtkImageData *data);

		float GetDataShift() const { return dataShift; }
		float GetDataScale() const { return dataScale; }
		Vector3 GetOriginalSize() const { return originalSize; }
		Vector3 GetOrigin() const { return origin; }

	protected:
		ImageDataTexture3D(int width, int height, int depth, 
			int internalformat);

	private:
		double dataShift;
		double dataScale;
		Vector3 origin;
		Vector3 originalSize;
	};
}
