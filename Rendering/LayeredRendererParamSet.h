#pragma once

#include "ParamSet.h"

#include "Camera.h"

namespace NQVTK
{
	class LayeredRendererParamSet : public ParamSet
	{
	public:
		LayeredRendererParamSet()
		{
			layer = 0;
			camera = 0;
			viewportX = viewportY = 0;
		}

		virtual void SetupProgram(GLProgram *program)
		{
			program->SetUniform1i("layer", layer);
			program->SetUniform1f("farPlane", camera->farZ);
			program->SetUniform1f("nearPlane", camera->nearZ);
			program->SetUniform1f("viewportX", viewportX);
			program->SetUniform1f("viewportY", viewportY);
		}

		int layer;
		NQVTK::Camera *camera;
		float viewportX;
		float viewportY;

	private:
		// Not implemented
		LayeredRendererParamSet(const LayeredRendererParamSet&);
		void operator=(const LayeredRendererParamSet&);
	};
}
