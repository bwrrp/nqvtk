#include "FilteredVolumeParamSet.h"

#include "Rendering/Volume.h"

#include "GPGPU/VolumeGaussianFilter.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	FilteredVolumeParamSet::FilteredVolumeParamSet(
		Volume *volume, GPGPU::VolumeToVolumeFilter *filter)
		: VolumeParamSet(0), originalVolume(0), 
		filter(filter), filterEnabled(true)
	{
		assert(volume);
		assert(filter);

		// NOTE: don't subclass and make SetVolume virtual again!
		SetVolume(volume);

		SetFilterEnabled(false);
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

		if (filterEnabled)
		{
			// Get rid of the filtered volume
			delete this->volume;
			this->volume = 0;
		}

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
	void FilteredVolumeParamSet::SetFilterEnabled(bool enabled)
	{
		if (enabled == filterEnabled) return;

		if (!filter)
		{
			filterEnabled = false;
			return;
		}

		filterEnabled = enabled;

		if (enabled)
		{
			// Turn on filtering
			volume = 0;
			Update();
		}
		else
		{
			// Delete filtered volume
			delete volume;
			volume = originalVolume;
		}
	}

	// ------------------------------------------------------------------------
	void FilteredVolumeParamSet::Update()
	{
		if (filterEnabled)
		{
			NQVTK::GPGPU::VolumeGaussianFilter *scalefilter = 
				dynamic_cast<NQVTK::GPGPU::VolumeGaussianFilter*>(filter);
			if (scalefilter) scalefilter->scale = scale;
			volume = filter->Execute(volume);
		}
	}
}
