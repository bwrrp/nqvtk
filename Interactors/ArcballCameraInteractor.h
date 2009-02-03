#pragma once

#include "Interactor.h"

#include "Rendering/ArcballCamera.h"

namespace NQVTK
{
	class ArcballCameraInteractor : public Interactor
	{
	public:
		typedef Interactor Superclass;

		ArcballCameraInteractor(NQVTK::ArcballCamera *camera)
		{
			assert(camera);
			this->camera = camera;
		}

	protected:
		NQVTK::ArcballCamera *camera;

	private:
		// Not implemented
		ArcballCameraInteractor(const ArcballCameraInteractor&);
		void operator=(const ArcballCameraInteractor&);
	};
}
