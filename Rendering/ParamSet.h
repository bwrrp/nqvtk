#pragma once

#include "GLBlaat/GLProgram.h"

#include <sstream>
#include <string>

namespace NQVTK
{
	class ParamSet
	{
	public:
		ParamSet() { };

		virtual void SetupProgram(GLProgram *program) { }
		virtual void SetupProgramArrays(GLProgram *program, int objectId) { }

	protected:
		std::string GetArrayName(const std::string &baseName, int index)
		{
			std::ostringstream name;
			name << baseName << "[" << index << "]";
			return name.str();
		}

	private:
		// Not implemented
		ParamSet(const ParamSet&);
		void operator=(const ParamSet&);
	};
}
