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

		virtual void SetupProgram(GLProgram *program)
		{
			// TODO: find a way to make param sets useful in the raycaster
			// Properties for all objects need to be available simultaneously 
			// in the painter. Paramsets could write to arrays var[objectId]...
			//program->SetUniform1f("tfStart", tfStart);
			//program->SetUniform1f("tfEnd", tfEnd);
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
