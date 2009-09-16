#pragma once

#include "CameraInteractor.h"

namespace NQVTK
{
	class OrbitCamera;

	class OrbitCameraInteractor : public CameraInteractor
	{
	public:
		typedef CameraInteractor Superclass;

		OrbitCameraInteractor(OrbitCamera *orbitCam);

		virtual bool MouseMoveEvent(MouseEvent event);

	protected:
		// Previous mouse coordinates
		int lastX;
		int lastY;

	private:
		// Not implemented
		OrbitCameraInteractor(const OrbitCameraInteractor&);
		void operator=(const OrbitCameraInteractor&);
	};
}
