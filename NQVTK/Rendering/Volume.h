#pragma once

#include "GLBlaat/GLTexture3D.h"

#include "NQVTK/Math/Vector3.h"

namespace NQVTK
{
	class Volume : public GLTexture3D
	{
	public:
		typedef GLTexture3D Superclass;

		static Volume *New(int width, int height, int depth, 
			int internalformat, int format, int type, void *data);

		// Getters for metadata
		double GetDataShift() const { return dataShift; }
		double GetDataScale() const { return dataScale; }
		Vector3 GetOriginalSize() const { return originalSize; }
		Vector3 GetOrigin() const { return origin; }

		// Setters for metadata
		void SetDataShift(double value) { dataShift = value; }
		void SetDataScale(double value) { dataScale = value; }
		void SetOriginalSize(const Vector3 &value) { originalSize = value; }
		void SetOrigin(const Vector3 &value) { origin = value; }

	protected:
		double dataShift;
		double dataScale;
		Vector3 origin;
		Vector3 originalSize;

		Volume(int width, int height, int depth, 
			int internalformat);
	};
}
