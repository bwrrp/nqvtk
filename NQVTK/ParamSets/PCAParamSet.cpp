#include "PCAParamSet.h"

#include "Renderables/VBOMesh.h"

#include "GLBlaat/GLProgram.h"

#include <vector>
#include <string>
#include <sstream>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	PCAParamSet::PCAParamSet(int numEigenModes)
	{
		weights.resize(numEigenModes);
	}

	// ------------------------------------------------------------------------
	void PCAParamSet::SetupProgram(GLProgram *program)
	{
		SetupProgram(program, "weights");
	}

	// ------------------------------------------------------------------------
	void PCAParamSet::SetupProgram(GLProgram *program, 
		const std::string &varname)
	{
		for (unsigned int i = 0; i < weights.size(); ++i)
		{
			std::ostringstream name;
			name << varname << "[" << i << "]";
			program->SetUniform1f(name.str(), weights[i]);
		}
	}

	// ------------------------------------------------------------------------
	int PCAParamSet::GetNumEigenModes(NQVTK::Renderable *object)
	{
		NQVTK::VBOMesh *mesh = dynamic_cast<NQVTK::VBOMesh*>(object);
		if (!mesh) return 0;

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
}
