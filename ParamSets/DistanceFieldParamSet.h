#pragma once

#include "ParamSet.h"

namespace NQVTK
{
	class ImageDataTexture3D;

	class DistanceFieldParamSet : public ParamSet
	{
	public:
		typedef ParamSet Superclass;

		DistanceFieldParamSet(ImageDataTexture3D *distanceField = 0);
		virtual ~DistanceFieldParamSet();

		virtual void SetupProgram(GLProgram *program);
		virtual void SetupProgramArrays(GLProgram *program, int objectId);

		ImageDataTexture3D *distanceField;

	private:
		// Not implemented
		DistanceFieldParamSet(const DistanceFieldParamSet&);
		void operator=(const DistanceFieldParamSet&);
	};
}
