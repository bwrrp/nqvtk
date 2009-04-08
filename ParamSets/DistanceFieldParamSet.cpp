#pragma once

#include "DistanceFieldParamSet.h"

#include "Rendering/ImageDataTexture3D.h"

#include "GLBlaat/GLProgram.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	DistanceFieldParamSet::DistanceFieldParamSet(
		ImageDataTexture3D *distanceField)
	{
		this->distanceField = distanceField;
	}

	// ------------------------------------------------------------------------
	DistanceFieldParamSet::~DistanceFieldParamSet()
	{
		delete distanceField;
	}

	// ------------------------------------------------------------------------
	void DistanceFieldParamSet::SetupProgram(GLProgram *program)
	{
		if (distanceField)
		{
			program->SetUniform1i("hasDistanceField", 1);
			program->SetUniform1f("distanceFieldDataShift", 
				static_cast<float>(distanceField->GetDataShift()));
			program->SetUniform1f("distanceFieldDataScale", 
				static_cast<float>(distanceField->GetDataScale()));
			Vector3 origin = distanceField->GetOrigin();
			program->SetUniform3f("distanceFieldOrigin", 
				static_cast<float>(origin.x), 
				static_cast<float>(origin.y), 
				static_cast<float>(origin.z));
			Vector3 size = distanceField->GetOriginalSize();
			program->SetUniform3f("distanceFieldSize", 
				static_cast<float>(size.x), 
				static_cast<float>(size.y), 
				static_cast<float>(size.z));
		}
		else
		{
			program->SetUniform1i("hasDistanceField", 0);
		}
	}

	// ------------------------------------------------------------------------
	void DistanceFieldParamSet::SetupProgramArrays(
		GLProgram *program, int objectId)
	{
		// TODO: maybe we should just rename this class...
		program->SetUniform1f(
			GetArrayName("volumeDataShift", objectId), 
			static_cast<float>(distanceField->GetDataShift()));
		program->SetUniform1f(
			GetArrayName("volumeDataScale", objectId), 
			static_cast<float>(distanceField->GetDataScale()));
		NQVTK::Vector3 origin = distanceField->GetOrigin();
		program->SetUniform3f(
			GetArrayName("volumeOrigin", objectId), 
			static_cast<float>(origin.x), 
			static_cast<float>(origin.y), 
			static_cast<float>(origin.z));
		NQVTK::Vector3 size = distanceField->GetOriginalSize();
		program->SetUniform3f(
			GetArrayName("volumeSize", objectId), 
			static_cast<float>(size.x), 
			static_cast<float>(size.y), 
			static_cast<float>(size.z));
		program->SetUniform3f(
			GetArrayName("volumeSpacing", objectId), 
			static_cast<float>(
				size.x / static_cast<double>(distanceField->GetWidth() - 1)), 
			static_cast<float>(
				size.y / static_cast<double>(distanceField->GetHeight() - 1)), 
			static_cast<float>(
				size.z / static_cast<double>(distanceField->GetDepth() - 1)));
	}
}
