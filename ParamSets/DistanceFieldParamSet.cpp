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
				distanceField->GetDataShift());
			program->SetUniform1f("distanceFieldDataScale", 
				distanceField->GetDataScale());
			Vector3 origin = distanceField->GetOrigin();
			program->SetUniform3f("distanceFieldOrigin", 
				origin.x, origin.y, origin.z);
			Vector3 size = distanceField->GetOriginalSize();
			program->SetUniform3f("distanceFieldSize", 
				size.x, size.y, size.z);
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
			distanceField->GetDataShift());
		program->SetUniform1f(
			GetArrayName("volumeDataScale", objectId), 
			distanceField->GetDataScale());
		NQVTK::Vector3 origin = distanceField->GetOrigin();
		program->SetUniform3f(
			GetArrayName("volumeOrigin", objectId), 
			origin.x, origin.y, origin.z);
		NQVTK::Vector3 size = distanceField->GetOriginalSize();
		program->SetUniform3f(
			GetArrayName("volumeSize", objectId), 
			size.x, size.y, size.z);
		program->SetUniform3f(
			GetArrayName("volumeSpacing", objectId), 
			size.x / static_cast<double>(distanceField->GetWidth() - 1), 
			size.y / static_cast<double>(distanceField->GetHeight() - 1), 
			size.z / static_cast<double>(distanceField->GetDepth() - 1));
	}
}
