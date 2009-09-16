#pragma once

#include "VolumeToVolumeFilter.h"

namespace NQVTK
{
	namespace GPGPU
	{
		class VolumeJacobianDeterminant : public VolumeToVolumeFilter
		{
		public:
			typedef VolumeToVolumeFilter Superclass;

			VolumeJacobianDeterminant();
			virtual ~VolumeJacobianDeterminant();

			virtual bool Setup(Volume *input);

			virtual Volume *Execute(Volume *outVol = 0);

			float scale;
			float outDataShift;
			float outDataScale;

		protected:
			virtual GLProgram *CreateProgram();
			virtual void SetupProgramParameters(GLProgram *program);
		};
	}
}
