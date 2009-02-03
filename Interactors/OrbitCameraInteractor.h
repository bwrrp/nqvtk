#pragma once

#include "Interactor.h"

#include "Rendering/OrbitCamera.h"

namespace NQVTK
{
	class OrbitCameraInteractor : public NQVTK::Interactor
	{
	public:
		typedef Interactor Superclass;

		OrbitCameraInteractor(NQVTK::OrbitCamera *orbitCam)
		{
			// TODO: remove dependency on OrbitCamera
			// TODO: move OrbitCamera logic to the interactor
			// TODO: add an ArcballCameraInteractor
			assert(orbitCam);
			camera = orbitCam;
			lastX = lastY = 0;
		}

		virtual bool MouseMoveEvent(QMouseEvent *event)
		{
			bool handled = false;

			// Mouse controls camera
			if (event->buttons() & Qt::LeftButton)
			{
				// Rotate
				camera->rotateY += event->x() - lastX;
				camera->rotateX -= event->y() - lastY;
				if (camera->rotateX > 80.0) camera->rotateX = 80.0;
				if (camera->rotateX < -80.0) camera->rotateX = -80.0;
				handled = true;
			}

			if (event->buttons() & Qt::RightButton)
			{
				// Zoom
				camera->zoom += (event->y() - lastY) * 0.01f;
				if (camera->zoom < 0.05) camera->zoom = 0.05;
				if (camera->zoom > 20.0) camera->zoom = 20.0;
				handled = true;
			}

			if (handled) camera->Update();

			lastX = event->x();
			lastY = event->y();

			return handled;
		}

	protected:
		NQVTK::OrbitCamera *camera;

		// Previous mouse coordinates
		int lastX;
		int lastY;

	private:
		// Not implemented
		OrbitCameraInteractor(const OrbitCameraInteractor&);
		void operator=(const OrbitCameraInteractor&);
	};
}
