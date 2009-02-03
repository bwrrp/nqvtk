#pragma once

#include "Interactor.h"

#include "Rendering/Camera.h"

namespace NQVTK
{
	class CameraInteractor : public Interactor
	{
	public:
		typedef Interactor Superclass;

		CameraInteractor(Camera *camera) : camera(camera) { }

	protected:
		Camera *camera;
	};
}
