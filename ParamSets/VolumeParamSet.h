#pragma once

#include "ParamSet.h"

namespace NQVTK
{
	class ImageDataTexture3D;

	class VolumeParamSet : public ParamSet
	{
	public:
		typedef ParamSet Superclass;

		VolumeParamSet(ImageDataTexture3D *volume = 0);
		virtual ~VolumeParamSet();

		virtual void SetupProgram(GLProgram *program);
		virtual void SetupProgramArrays(GLProgram *program, int objectId);

		ImageDataTexture3D *volume;

	private:
		// Not implemented
		VolumeParamSet(const VolumeParamSet&);
		void operator=(const VolumeParamSet&);
	};
}
