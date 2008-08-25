#pragma once

#include "ParamSet.h"

#include "Rendering/VBOMesh.h"

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

		static int GetNumEigenModes(NQVTK::VBOMesh *mesh)
		{
			// We assume a shape model mesh contains attribs called eigvecs[i]
			int i = 0;
			NQVTK::AttributeSet *aset;
			do
			{
				std::ostringstream name;
				name << "eigvecs[" << i << "]";
				aset = mesh->GetAttributeSet(name.str());
				++i;
			}
			while (aset);
			return i - 1;
		}

		std::vector<float> weights;

	private:
		// Not implemented
		PCAParamSet(const PCAParamSet&);
		void operator=(const PCAParamSet&);
	};
}
