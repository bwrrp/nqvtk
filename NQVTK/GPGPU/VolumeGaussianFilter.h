#pragma once

#include "VolumeToVolumeFilter.h"

namespace NQVTK
{
	namespace GPGPU
	{
		class VolumeGaussianFilter : public VolumeToVolumeFilter
		{
		public:
			typedef VolumeToVolumeFilter Superclass;

			VolumeGaussianFilter();
			virtual ~VolumeGaussianFilter();

			virtual bool Setup(Volume *input);

			virtual Volume *Execute(Volume *outVol = 0);

			float scale;

		protected:
			Volume *helperYZX;
			Volume *helperZXY;

			virtual GLProgram *CreateProgram();
			virtual void SetupProgramParameters(GLProgram *program);
		};
	}
}
