#pragma once

#include "ParamSet.h"

namespace NQVTK
{
	class TransferFunctionParamSet : public ParamSet
	{
	public:
		TransferFunctionParamSet()
		{
			tfStart = 0.0;
			tfEnd = 1.0;
			// TODO: use transfer function texture (optional?)
		}

		virtual ~TransferFunctionParamSet()
		{
		}

		virtual void SetupProgramArrays(GLProgram *program, int objectId)
		{
			program->SetUniform1f(GetArrayName("tfStart", objectId), tfStart);
			program->SetUniform1f(GetArrayName("tfEnd", objectId), tfEnd);
			// TODO: set transfer function texture if available
		}

		float tfStart;
		float tfEnd;

	private:
		// Not implemented
		TransferFunctionParamSet(const TransferFunctionParamSet&);
		void operator=(const TransferFunctionParamSet&);
	};
}
