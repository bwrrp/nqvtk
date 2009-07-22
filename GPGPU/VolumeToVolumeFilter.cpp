#include "VolumeToVolumeFilter.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTextureManager.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	namespace GPGPU
	{
		// --------------------------------------------------------------------
		VolumeToVolumeFilter::VolumeToVolumeFilter()
			: tm(0), program(0), input(0)
		{
		}

		// --------------------------------------------------------------------
		VolumeToVolumeFilter::~VolumeToVolumeFilter()
		{
			delete tm;
			delete program;
		}

		// --------------------------------------------------------------------
		bool VolumeToVolumeFilter::Setup(Volume *input)
		{
            tm = GLTextureManager::New();

			assert(input);
			this->input = input;

			std::cout << "Creating filter..." << std::endl;
			program = CreateProgram();
			if (!program)
			{
				std::cerr << "Could not create filter program!" << std::endl;
				return false;
			}

			return true;
		}
	}
}
