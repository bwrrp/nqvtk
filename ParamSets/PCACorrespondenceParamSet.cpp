#pragma once

#include "PCACorrespondenceParamSet.h"

#include "PCAParamSet.h"

#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	PCACorrespondenceParamSet::PCACorrespondenceParamSet(PCAParamSet *set1, PCAParamSet *set2)
		: set1(set1), set2(set2) 
	{
		assert(set1);
		assert(set2);
	}

	// ------------------------------------------------------------------------
	void PCACorrespondenceParamSet::SetupProgram(GLProgram *program)
	{
		set1->SetupProgram(program, "weights");
		set2->SetupProgram(program, "weights2");
	}
}
