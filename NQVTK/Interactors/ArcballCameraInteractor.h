#pragma once

#include "CameraInteractor.h"

#include "NQVTK/Math/Vector3.h"

namespace NQVTK
{
	class ArcballCamera;

	class ArcballCameraInteractor : public CameraInteractor
	{
	public:
		typedef Interactor Superclass;

		ArcballCameraInteractor(NQVTK::ArcballCamera *arcballCam);

		virtual bool MouseMoveEvent(MouseEvent event);
		virtual bool MousePressEvent(MouseEvent event);
		virtual bool MouseReleaseEvent(MouseEvent event);

	protected:
		// Previous mouse coordinates
		int lastX;
		int lastY;

		Vector3 GetBallPoint(int x, int y);

	private:
		// Not implemented
		ArcballCameraInteractor(const ArcballCameraInteractor&);
		void operator=(const ArcballCameraInteractor&);
	};
}
