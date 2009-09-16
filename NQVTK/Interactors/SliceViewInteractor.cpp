#include "SliceViewInteractor.h"

#include "Rendering/SliceRenderer.h"
#include "Rendering/View.h"

#include "Math/Vector3.h"

#include <algorithm>
#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	bool PointInBoundingBox(const Vector3 &point, const double *bounds)
	{
		return point.x >= bounds[0] && point.x <= bounds[1] &&
			point.y >= bounds[2] && point.y <= bounds[3] &&
			point.z >= bounds[4] && point.z <= bounds[5];
	}

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
			Vector3 sliceNormal = right.cross(up).normalized();

			// Figure out the extent of the data along axis
			Vector3 corner(
				sliceNormal.x > 0 ? bounds[1] : bounds[0], 
				sliceNormal.y > 0 ? bounds[3] : bounds[2], 
				sliceNormal.z > 0 ? bounds[5] : bounds[4]);
			Vector3 halfDiagonal = corner - center;
			double extent = halfDiagonal.dot(sliceNormal);

			// Figure out the current plane position along this axis
			double planeOffset = (origin - center).dot(sliceNormal);

			// Mouse movement moves the position along the axis
			double scale = static_cast<double>(
				std::min(viewportWidth, viewportHeight));
			double delta = extent * (lastY - event.y) / scale;

			// Keep the plane intersecting the bounding box
			if (planeOffset + delta > extent)
			{
				delta = extent - planeOffset;
			}
			else if (planeOffset + delta < -extent)
			{
				delta = -extent - planeOffset;
			}
			Vector3 offset = sliceNormal * delta;

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
			Vector3 offset = offsetX + offsetY;

			// We try to keep the slice center within the bounding box
			Vector3 sliceCenter = origin + 0.5 * right + 0.5 * up;
			Vector3 sliceNormal = right.cross(up).normalized();

			if (PointInBoundingBox(sliceCenter, bounds))
			{
				// Clip movement to bounding box
				// TODO: sometimes clipped positions are still outside, why?
				// (accuracy problem?)
				Vector3 newSliceCenter = sliceCenter + offset;
				newSliceCenter.x = std::max(bounds[0], 
					std::min(bounds[1], newSliceCenter.x));
				newSliceCenter.y = std::max(bounds[2], 
					std::min(bounds[3], newSliceCenter.y));
				newSliceCenter.z = std::max(bounds[4], 
					std::min(bounds[5], newSliceCenter.z));
				offset = newSliceCenter - sliceCenter;
				// Restrict movement to slice plane
				offset = offset - sliceNormal.dot(offset) * sliceNormal;
			}
			else
			{
				// Only allow movement towards the bounding box
				Vector3 gradient;
				Vector3 newSliceCenter = sliceCenter + offset;
				for (int i = 0; i < 3; ++i)
				{
					if (newSliceCenter.V[i] < bounds[2*i])
					{
						gradient.V[i] = newSliceCenter.V[i] - bounds[2*i];
					}
					else if (newSliceCenter.V[i] > bounds[2*i+1])
					{
						gradient.V[i] = newSliceCenter.V[i] - bounds[2*i+1];
					}
				}
				// Only consider the in-slice part of this vector
				gradient = (gradient - 
					sliceNormal.dot(gradient) * sliceNormal).normalized();
				double dot = gradient.dot(offset);
				if (dot > 0)
				{
					// Remove the component moving away from the bounding box
					offset = offset - dot * gradient;
				}
			}

			Vector3 neworigin = origin + offset;

			renderer->SetPlane(neworigin, right, up);

			handled = true;
		}

		lastX = event.x;
		lastY = event.y;

		if (!handled) handled = Superclass::MouseMoveEvent(event);
		return handled;
	}
}
