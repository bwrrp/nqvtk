#pragma once

#include "GLBlaat/GLProgram.h"

namespace NQVTK
{
	class ParamSet
	{
	public:
		ParamSet() { };

		virtual void SetupProgram(GLProgram *program) = 0;

	private:
		// Not implemented
		ParamSet(const ParamSet&);
		void operator=(const ParamSet&);
	};
}
