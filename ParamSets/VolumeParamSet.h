#pragma once

#include "ParamSet.h"

namespace NQVTK
{
	class Volume;

	class VolumeParamSet : public ParamSet
	{
	public:
		typedef ParamSet Superclass;

		VolumeParamSet(Volume *volume = 0);
		virtual ~VolumeParamSet();

		virtual void SetupProgram(GLProgram *program);
		virtual void SetupProgramArrays(GLProgram *program, int objectId);

		Volume *volume;

	private:
		// Not implemented
		VolumeParamSet(const VolumeParamSet&);
		void operator=(const VolumeParamSet&);
	};
}
