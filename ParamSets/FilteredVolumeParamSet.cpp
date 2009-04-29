#pragma once

#include "FilteredVolumeParamSet.h"

#include "Rendering/Volume.h"

#include "GPGPU/VolumeToVolumeFilter.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	FilteredVolumeParamSet::FilteredVolumeParamSet(
		Volume *volume, GPGPU::VolumeToVolumeFilter *filter)
		: VolumeParamSet(0), originalVolume(0), filter(filter)
	{
		assert(volume);
		assert(filter);

		// NOTE: calling a virtual function here won't call overrides!
		// Don't subclass this paramset!
		SetVolume(volume);
	}

	// ------------------------------------------------------------------------
	FilteredVolumeParamSet::~FilteredVolumeParamSet()
	{
		delete originalVolume;
		delete filter;
	}

	// ------------------------------------------------------------------------
	Volume *FilteredVolumeParamSet::SetVolume(Volume *volume)
	{
		Volume *old = originalVolume;
		// Don't return an old volume if it's not actually replaced
		if (originalVolume == volume) old = 0;

		originalVolume = volume;

		// Get rid of the filtered volume
		delete this->volume;
		this->volume = 0;

		if (filter)
		{
			// Setup the filter
			bool res = filter->Setup(volume);
			if (!res)
			{
				std::cerr << "Error setting up filter" << std::endl;
				delete filter;
				filter = 0;
			}
		}

		return old;
	}

	// ------------------------------------------------------------------------
	void FilteredVolumeParamSet::Update(float scale)
	{
		filter->scale = scale;
		volume = filter->Execute(volume);
	}
}
