#pragma once

#include "GLBlaat/GLTexture3D.h"

#include "Math/Vector3.h"

namespace NQVTK
{
	class Volume : public GLTexture3D
	{
	public:
		typedef GLTexture3D Superclass;

		static Volume *New();

		double GetDataShift() const { return dataShift; }
		double GetDataScale() const { return dataScale; }
		Vector3 GetOriginalSize() const { return originalSize; }
		Vector3 GetOrigin() const { return origin; }

	protected:
		double dataShift;
		double dataScale;
		Vector3 origin;
		Vector3 originalSize;

		Volume(int width, int height, int depth, 
			int internalformat);
	};
}
