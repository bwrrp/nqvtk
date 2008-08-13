#pragma once

#include "ParamSet.h"

#include <vector>
#include <string>
#include <sstream>

namespace NQVTK
{
	class PCAParamSet : public ParamSet
	{
	public:
		PCAParamSet(int numEigenModes)
		{
			weights.resize(numEigenModes);
		}

		virtual void SetupProgram(GLProgram *program)
		{
			for (unsigned int i = 0; i < weights.size(); ++i)
			{
				std::ostringstream name;
				name << "weights[" << i << "]";
				program->SetUniform1f(name.str(), weights[i]);
			}
		}

		std::vector<float> weights;

	private:
		// Not implemented
		PCAParamSet(const PCAParamSet&);
		void operator=(const PCAParamSet&);
	};
}
