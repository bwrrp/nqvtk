#pragma once

#include "ParamSet.h"

namespace NQVTK
{
	class TransferFunctionParamSet : public ParamSet
	{
	public:
		TransferFunctionParamSet();
		virtual ~TransferFunctionParamSet();

		virtual void SetupProgramArrays(GLProgram *program, int objectId);

		float tfStart;
		float tfEnd;

	private:
		// Not implemented
		TransferFunctionParamSet(const TransferFunctionParamSet&);
		void operator=(const TransferFunctionParamSet&);
	};
}
