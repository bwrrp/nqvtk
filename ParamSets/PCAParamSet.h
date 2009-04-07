#pragma once

#include "ParamSet.h"

#include <string>
#include <vector>

namespace NQVTK
{
	class VBOMesh;

	class PCAParamSet : public ParamSet
	{
	public:
		PCAParamSet(int numEigenModes);

		virtual void SetupProgram(GLProgram *program);
		virtual void SetupProgram(GLProgram *program, const std::string &varname);

		static int GetNumEigenModes(NQVTK::VBOMesh *mesh);

		std::vector<float> weights;

	private:
		// Not implemented
		PCAParamSet(const PCAParamSet&);
		void operator=(const PCAParamSet&);
	};
}
