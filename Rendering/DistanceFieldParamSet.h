#pragma once

#include "ParamSet.h"

#include "ImageDataTexture3D.h"

namespace NQVTK
{
	class DistanceFieldParamSet : public ParamSet
	{
	public:
		DistanceFieldParamSet(ImageDataTexture3D *distanceField = 0)
		{
			this->distanceField = distanceField;
		}

		virtual ~DistanceFieldParamSet()
		{
			if (distanceField) delete distanceField;
		}

		virtual void SetupProgram(GLProgram *program)
		{
			if (distanceField)
			{
				program->SetUniform1i("hasDistanceField", 1);
				program->SetUniform1f("distanceFieldDataShift", 
					distanceField->GetDataShift());
				program->SetUniform1f("distanceFieldDataScale", 
					distanceField->GetDataScale());
				Vector3 origin = distanceField->GetOrigin();
				program->SetUniform3f("distanceFieldOrigin", 
					origin.x, origin.y, origin.z);
				Vector3 size = distanceField->GetOriginalSize();
				program->SetUniform3f("distanceFieldSize", 
					size.x, size.y, size.z);
			}
			else
			{
				program->SetUniform1i("hasDistanceField", 0);
			}
		}

		ImageDataTexture3D *distanceField;

	private:
		// Not implemented
		DistanceFieldParamSet(const DistanceFieldParamSet&);
		void operator=(const DistanceFieldParamSet&);
	};
}
