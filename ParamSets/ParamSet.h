#pragma once

#include <string>

class GLProgram;

namespace NQVTK
{
	class ParamSet
	{
	public:
		ParamSet();

		virtual void SetupProgram(GLProgram *program);
		virtual void SetupProgramArrays(GLProgram *program, int objectId);

	protected:
		std::string GetArrayName(const std::string &baseName, int index);

	private:
		// Not implemented
		ParamSet(const ParamSet&);
		void operator=(const ParamSet&);
	};
}
