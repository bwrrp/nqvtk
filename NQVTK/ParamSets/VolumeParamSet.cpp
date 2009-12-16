#include "VolumeParamSet.h"

#include "Rendering/Volume.h"

#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTextureManager.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	VolumeParamSet::VolumeParamSet(Volume *volume, const std::string &name)
		: volume(volume), name(name)
	{
	}

	// ------------------------------------------------------------------------
	VolumeParamSet::~VolumeParamSet()
	{
		delete volume;
	}

	// ------------------------------------------------------------------------
	void VolumeParamSet::SetupProgram(GLProgram *program)
	{
		if (volume)
		{
			// TODO: revise the param(set) stuff to make this more efficient

			// For legacy code
			program->SetUniform1i("hasVolume", 1);
			program->SetUniform1i(name + "Present", 1);
			program->SetUniform1f(name + "DataShift", 
				static_cast<float>(volume->GetDataShift()));
			program->SetUniform1f(name + "DataScale", 
				static_cast<float>(volume->GetDataScale()));
			Vector3 origin = volume->GetOrigin();
			program->SetUniform3f(name + "Origin", 
				static_cast<float>(origin.x), 
				static_cast<float>(origin.y), 
				static_cast<float>(origin.z));
			Vector3 size = volume->GetOriginalSize();
			program->SetUniform3f(name + "Size", 
				static_cast<float>(size.x), 
				static_cast<float>(size.y), 
				static_cast<float>(size.z));
			program->SetUniform3f(name + "Spacing", 
				static_cast<float>(
					size.x / static_cast<double>(volume->GetWidth() - 1)), 
				static_cast<float>(
					size.y / static_cast<double>(volume->GetHeight() - 1)), 
				static_cast<float>(
					size.z / static_cast<double>(volume->GetDepth() - 1)));
		}
		else
		{
			// For legacy code
			program->SetUniform1i("hasVolume", 0);
			program->SetUniform1i(name + "Present", 0);
		}
	}

	// ------------------------------------------------------------------------
	void VolumeParamSet::SetupProgramArrays(
		GLProgram *program, int objectId)
	{
		if (volume)
		{
			program->SetUniform1f(
				GetArrayName(name + "DataShift", objectId), 
				static_cast<float>(volume->GetDataShift()));
			program->SetUniform1f(
				GetArrayName(name + "DataScale", objectId), 
				static_cast<float>(volume->GetDataScale()));
			NQVTK::Vector3 origin = volume->GetOrigin();
			program->SetUniform3f(
				GetArrayName(name + "Origin", objectId), 
				static_cast<float>(origin.x), 
				static_cast<float>(origin.y), 
				static_cast<float>(origin.z));
			NQVTK::Vector3 size = volume->GetOriginalSize();
			program->SetUniform3f(
				GetArrayName(name + "Size", objectId), 
				static_cast<float>(size.x), 
				static_cast<float>(size.y), 
				static_cast<float>(size.z));
			program->SetUniform3f(
				GetArrayName(name + "Spacing", objectId), 
				static_cast<float>(
					size.x / static_cast<double>(volume->GetWidth() - 1)), 
				static_cast<float>(
					size.y / static_cast<double>(volume->GetHeight() - 1)), 
				static_cast<float>(
					size.z / static_cast<double>(volume->GetDepth() - 1)));
		}
	}

	// ------------------------------------------------------------------------
	void VolumeParamSet::SetupTextures(GLTextureManager *tm)
	{
		tm->AddTexture(name, volume, false);
	}

	// ------------------------------------------------------------------------
	void VolumeParamSet::SetupTextureArrays(GLTextureManager *tm, int objectId)
	{
		tm->AddTexture(GetArrayName(name, objectId), volume, false);
	}

	// ------------------------------------------------------------------------
	Volume *VolumeParamSet::SetVolume(Volume *volume)
	{
		if (this->volume == volume) return 0;

		Volume *old = this->volume;
		this->volume = volume;
		return old;
	}
}
