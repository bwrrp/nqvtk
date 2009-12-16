#pragma once

#include "ParamSet.h"

namespace NQVTK
{
	class Volume;

	class VolumeParamSet : public ParamSet
	{
	public:
		typedef ParamSet Superclass;

		VolumeParamSet(Volume *volume = 0, const std::string &name = "volume");
		virtual ~VolumeParamSet();

		virtual void SetupProgram(GLProgram *program);
		virtual void SetupProgramArrays(GLProgram *program, int objectId);

		virtual void SetupTextures(GLTextureManager *tm);
		virtual void SetupTextureArrays(GLTextureManager *tm, int objectId);

		virtual Volume *SetVolume(Volume *volume);
		Volume *GetVolume() { return volume; }

	protected:
		Volume *volume;
		std::string name;

	private:
		// Not implemented
		VolumeParamSet(const VolumeParamSet&);
		void operator=(const VolumeParamSet&);
	};
}
