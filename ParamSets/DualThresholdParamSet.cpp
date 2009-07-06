#include "DualThresholdParamSet.h"

#include "GLBlaat/GLProgram.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	DualThresholdParamSet::DualThresholdParamSet()
	{
		start = 0.0;
		end = 1.0;
	}

	// ------------------------------------------------------------------------
	DualThresholdParamSet::~DualThresholdParamSet()
	{
	}

	// ------------------------------------------------------------------------
	void DualThresholdParamSet::SetupProgramArrays(
		GLProgram *program, int objectId)
	{
		program->SetUniform1f(GetArrayName("tfStart", objectId), start);
		program->SetUniform1f(GetArrayName("tfEnd", objectId), end);
	}
}
