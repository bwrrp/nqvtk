#pragma once

#include "ParamSet.h"

#include <string>
#include <vector>

namespace NQVTK
{
	class Renderable;

	class PCAParamSet : public ParamSet
	{
	public:
		PCAParamSet(int numEigenModes);

		virtual void SetupProgram(GLProgram *program);
		virtual void SetupProgram(GLProgram *program, const std::string &varname);

		static int GetNumEigenModes(NQVTK::Renderable *object);

		std::vector<float> weights;

	private:
		// Not implemented
		PCAParamSet(const PCAParamSet&);
		void operator=(const PCAParamSet&);
	};
}
