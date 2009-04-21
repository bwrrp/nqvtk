#pragma once

#include "Volume.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	Volume *Volume::New()
	{
		// TODO...
		return 0;
	}

	// ------------------------------------------------------------------------
	Volume::Volume(int width, int height, int depth, 
		int internalformat) 
		: GLTexture3D(width, height, depth, internalformat) { }
}
