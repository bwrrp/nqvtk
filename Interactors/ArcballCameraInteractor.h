#pragma once

#include "CameraInteractor.h"

#include "Rendering/ArcballCamera.h"

#include "Math/Vector3.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace NQVTK
{
	class ArcballCameraInteractor : public CameraInteractor
	{
	public:
		typedef Interactor Superclass;

		ArcballCameraInteractor(NQVTK::ArcballCamera *arcballCam) 
			: CameraInteractor(arcballCam)
		{
			assert(arcballCam);
			lastX = lastY = 0;
		}

		virtual bool MouseMoveEvent(QMouseEvent *event)
		{
			ArcballCamera *camera = dynamic_cast<ArcballCamera*>(this->camera);
			assert(camera);

			bool handled = false;

			if (event->buttons() & Qt::LeftButton)
			{
				// Update arcball rotation
				Vector3 newPos = GetBallPoint(event->x(), event->y());
				camera->SetRotation(newPos);
				handled = true;
			}
			else if (event->buttons() & Qt::RightButton)
			{
				// Zoom
				double zoom = 1.0 + (event->y() - lastY) * 0.005f;
				if (zoom < 0.01) zoom = 0.01;
				Vector3 focusToPos = camera->position - camera->focus;
				focusToPos *= zoom;
				camera->position = camera->focus + focusToPos;
				handled = true;
			}

			lastX = event->x();
			lastY = event->y();

			if (!handled) return Superclass::MouseMoveEvent(event);
			return true;
		}

		virtual bool MousePressEvent(QMouseEvent *event)
		{
			ArcballCamera *camera = dynamic_cast<ArcballCamera*>(this->camera);
			assert(camera);
			
			if (event->button() == Qt::LeftButton)
			{
				// Start the arcball
				Vector3 startPos = GetBallPoint(event->x(), event->y());
				camera->StartArcball(startPos);
				return true;
			}

			return Superclass::MousePressEvent(event);
		}

		virtual bool MouseReleaseEvent(QMouseEvent *event)
		{
			ArcballCamera *camera = dynamic_cast<ArcballCamera*>(this->camera);
			assert(camera);

			if (event->button() == Qt::LeftButton)
			{
				// Stop the arcball
				camera->StopArcball();
				return true;
			}

			return Superclass::MouseReleaseEvent(event);
		}

	protected:
		// Previous mouse coordinates
		int lastX;
		int lastY;

		Vector3 GetBallPoint(int x, int y)
		{
			// Assume the ball is centered on the screen and has a 
			// screen-space diameter of 60% of the smallest viewport edge
			Vector3 center = Vector3(viewportWidth / 2.0, viewportHeight / 2.0, 0.0);
			double radius = 0.4 * std::min(viewportWidth, viewportHeight);

			// First assume z == 0
			Vector3 pos = Vector3(x, y, 0.0);
			
			// Compute relative position
			Vector3 relpos = (pos - center) / radius;

			double r = relpos.length();
			if (r > 1.0) 
			{
				// If the point is outside the ball, snap it to the horizon
				relpos = relpos / r;
			}
			else
			{
				// Otherwise, compute z
				relpos.z = sqrt(1.0 - r * r);
			}
			return relpos;
		}

	private:
		// Not implemented
		ArcballCameraInteractor(const ArcballCameraInteractor&);
		void operator=(const ArcballCameraInteractor&);
	};
}
