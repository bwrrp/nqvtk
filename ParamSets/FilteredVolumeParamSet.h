#pragma once

#include "VolumeParamSet.h"

namespace NQVTK
{
	class Volume;

	namespace GPGPU { class VolumeToVolumeFilter; }

	class FilteredVolumeParamSet : public VolumeParamSet
	{
	public:
		FilteredVolumeParamSet(Volume *volume, 
			GPGPU::VolumeToVolumeFilter *filter);
		~FilteredVolumeParamSet();

		Volume *SetVolume(Volume *volume);

		// TODO: pass the parameters in a generic way
		void Update(float scale);

	protected:
		Volume *originalVolume;
		GPGPU::VolumeToVolumeFilter *filter;

	private:
		// Not implemented
		FilteredVolumeParamSet(const FilteredVolumeParamSet&);
		void operator=(const FilteredVolumeParamSet&);
	};
}
