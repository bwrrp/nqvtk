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

		void SetFilterEnabled(bool enabled);
		bool GetFilterEnabled() { return filterEnabled; }

		void Update();

		// TODO: pass the parameters in a generic way
		float scale;

	protected:
		Volume *originalVolume;
		GPGPU::VolumeToVolumeFilter *filter;

		bool filterEnabled;

	private:
		// Not implemented
		FilteredVolumeParamSet(const FilteredVolumeParamSet&);
		void operator=(const FilteredVolumeParamSet&);
	};
}
