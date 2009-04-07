#pragma once

#include "TransferFunctionParamSet.h"

#include "GLBlaat/GLProgram.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	TransferFunctionParamSet::TransferFunctionParamSet()
	{
		tfStart = 0.0;
		tfEnd = 1.0;
		// TODO: use transfer function texture (optional?)
	}

	// ------------------------------------------------------------------------
	TransferFunctionParamSet::~TransferFunctionParamSet()
	{
	}

	// ------------------------------------------------------------------------
	void TransferFunctionParamSet::SetupProgramArrays(
		GLProgram *program, int objectId)
	{
		program->SetUniform1f(GetArrayName("tfStart", objectId), tfStart);
		program->SetUniform1f(GetArrayName("tfEnd", objectId), tfEnd);
		// TODO: set transfer function texture if available
	}
}
