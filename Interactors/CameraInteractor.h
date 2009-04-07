#pragma once

#include "Interactor.h"

namespace NQVTK
{
	class Camera;

	class CameraInteractor : public Interactor
	{
	public:
		typedef Interactor Superclass;

		CameraInteractor(Camera *camera);

	protected:
		Camera *camera;
	};
}
