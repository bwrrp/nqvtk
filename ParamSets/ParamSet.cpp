#pragma once

#include "ParamSet.h"

#include <sstream>
#include <string>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	ParamSet::ParamSet() 
	{ 
	}

	// ------------------------------------------------------------------------
	void ParamSet::SetupProgram(GLProgram *program)
	{
	}

	// ------------------------------------------------------------------------
	void ParamSet::SetupProgramArrays(GLProgram *program, int objectId)
	{
	}

	// ------------------------------------------------------------------------
	void ParamSet::SetupTextures(GLTextureManager *tm)
	{
	}

	// ------------------------------------------------------------------------
	void ParamSet::SetupTextureArrays(GLTextureManager *tm, int objectId)
	{
	}

	// ------------------------------------------------------------------------
	std::string ParamSet::GetArrayName(const std::string &baseName, int index)
	{
		std::ostringstream name;
		name << baseName << "[" << index << "]";
		return name.str();
	}
}
