#include "SliceViewInteractor.h"

#include "Rendering/SliceRenderer.h"
#include "Rendering/View.h"

#include "Math/Vector3.h"

#include <algorithm>
#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	SliceViewInteractor::SliceViewInteractor(SliceRenderer *renderer)
		: renderer(renderer)
	{
		assert(renderer);
	}
	
	// ------------------------------------------------------------------------
	bool SliceViewInteractor::MouseMoveEvent(MouseEvent event)
	{
		bool handled = false;

		// Get slice plane parameters
		Vector3 origin = renderer->GetPlaneOrigin();
		Vector3 right = renderer->GetPlaneRight();
		Vector3 up = renderer->GetPlaneUp();

		// Get scene info
		NQVTK::View *view = renderer->GetView();
		if (!view) return false;
		const double *bounds = view->GetVisibleBounds();
		Vector3 center = view->GetVisibleCenter();

		if (event.buttons & MouseEvent::LeftButton)
		{
			// Move slice plane perpendicular to itself
			Vector3 axis = right.cross(up).normalized();

			// Figure out the extent of the data along axis
			Vector3 diagonal(
				bounds[1] - bounds[0], 
				bounds[3] - bounds[2], 
				bounds[5] - bounds[4]);
			double extent = diagonal.dot(axis);

			// Figure out the current plane position along this axis
			double planeoffset = (origin - center).dot(axis);

			// Mouse movement moves the position along the axis
			double scale = static_cast<double>(
				std::min(viewportWidth, viewportHeight));
			double delta = extent * (lastY - event.y) / scale;

			// Keep the plane intersecting the bounding box
			if (planeoffset + delta > 0.5 * extent)
			{
				delta = 0.5 * extent - planeoffset;
			}
			if (planeoffset + delta < -0.5 * extent)
			{
				delta = -0.5 * extent - planeoffset;
			}
			Vector3 offset = axis * delta;

			// Move plane origin
			renderer->SetPlane(origin + offset, right, up);

			handled = true;
		}
		else if (event.buttons & MouseEvent::RightButton)
		{
			// Zoom into the center of the slice
			Vector3 sliceCenter = origin + 0.5 * right + 0.5 * up;
			double scale = static_cast<double>(
				std::min(viewportWidth, viewportHeight));
			double zoom = 1.0 + (lastY - event.y) / scale;

			// Try not to destroy the plane spanning vectors
			// They may still lose accuracy when zooming out too far...
			zoom = std::max(zoom, 0.01);

			Vector3 newright = zoom * right;
			Vector3 newup = zoom * up;
			Vector3 neworigin = sliceCenter - 0.5 * newright - 0.5 * newup;

			renderer->SetPlane(neworigin, newright, newup);

			handled = true;
		}
		else if (event.buttons & MouseEvent::MiddleButton)
		{
			// Pan slice
			double scale = static_cast<double>(
				std::min(viewportWidth, viewportHeight));
			double deltaX = (lastX - event.x) / scale;
			double deltaY = (event.y - lastY) / scale;

			double extentX = right.length();
			double extentY = up.length();

			Vector3 offsetX = deltaX * extentX * right.normalized();
			Vector3 offsetY = deltaY * extentY * up.normalized();

			// Keep the slice center within the bounding box
			// TODO: this should only pan the slice
			// TODO: the same check could be applied when changing slices
			Vector3 sliceCenter = origin + offsetX + offsetY + 
				0.5 * right + 0.5 * up;
			sliceCenter.x = std::max(bounds[0], 
				std::min(bounds[1], sliceCenter.x));
			sliceCenter.y = std::max(bounds[2], 
				std::min(bounds[3], sliceCenter.y));
			sliceCenter.z = std::max(bounds[4], 
				std::min(bounds[5], sliceCenter.z));
			Vector3 neworigin = sliceCenter - 0.5 * right - 0.5 * up;

			renderer->SetPlane(neworigin, right, up);

			handled = true;
		}

		lastX = event.x;
		lastY = event.y;

		if (!handled) handled = Superclass::MouseMoveEvent(event);
		return handled;
	}
}
