#pragma once

#include "ParamSet.h"

#include "PCAParamSet.h"

#include "GLBlaat/GLProgram.h"

#include <cassert>

namespace NQVTK
{
	class PCACorrespondenceParamSet : public ParamSet
	{
	public:
		typedef ParamSet Superclass;

		PCACorrespondenceParamSet(PCAParamSet *set1, PCAParamSet *set2)
			: set1(set1), set2(set2) 
		{
			assert(set1);
			assert(set2);
		}

		virtual void SetupProgram(GLProgram *program)
		{
			set1->SetupProgram(program, "weights");
			set2->SetupProgram(program, "weights2");
		}

	protected:
		PCAParamSet *set1;
		PCAParamSet *set2;
	};
}
