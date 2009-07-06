#include "Volume.h"

#include "GLBlaat/GLTexture3D.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	Volume *Volume::New(int width, int height, int depth, 
		int internalformat, int format, int type, void *data)
	{
		// TODO: use checks as in GLTexture3D
		// TODO: Maybe encapsulation is better here?

		// Create the Volume
		Volume *vol = new Volume(width, height, depth, internalformat);
		if (!vol->Allocate(format, type, data)) 
		{
			delete vol;
			return 0;
		}
		return vol;
	}

	// ------------------------------------------------------------------------
	Volume::Volume(int width, int height, int depth, int internalformat) 
		: GLTexture3D(width, height, depth, internalformat) 
	{
		// Set default metadata
		dataShift = 0.0;
		dataScale = 1.0;
		origin = Vector3();
		originalSize = Vector3(
			static_cast<double>(width), 
			static_cast<double>(height), 
			static_cast<double>(depth));
	}
}
