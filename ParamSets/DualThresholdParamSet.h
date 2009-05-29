#pragma once

#include "ParamSet.h"

namespace NQVTK
{
	class DualThresholdParamSet : public ParamSet
	{
	public:
		DualThresholdParamSet();
		virtual ~DualThresholdParamSet();

		virtual void SetupProgramArrays(GLProgram *program, int objectId);

		float start;
		float end;

	private:
		// Not implemented
		DualThresholdParamSet(const DualThresholdParamSet&);
		void operator=(const DualThresholdParamSet&);
	};
}
