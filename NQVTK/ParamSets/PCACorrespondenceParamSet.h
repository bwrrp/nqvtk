#pragma once

#include "ParamSet.h"

namespace NQVTK
{
	class PCAParamSet;

	class PCACorrespondenceParamSet : public ParamSet
	{
	public:
		typedef ParamSet Superclass;

		PCACorrespondenceParamSet(PCAParamSet *set1, PCAParamSet *set2);

		virtual void SetupProgram(GLProgram *program);

	protected:
		PCAParamSet *set1;
		PCAParamSet *set2;
	};
}
