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

		if (event.buttons & MouseEvent::LeftButton)
		{
			NQVTK::View *view = renderer->GetView();
			const double *bounds = view->GetVisibleBounds();

			// Get slice plane parameters
			Vector3 origin = renderer->GetPlaneOrigin();
			Vector3 right = renderer->GetPlaneRight();
			Vector3 up = renderer->GetPlaneUp();

			Vector3 axis = right.cross(up).normalized();

			// Figure out the extent of the data along axis
			Vector3 diagonal(
				bounds[1] - bounds[0], 
				bounds[3] - bounds[2], 
				bounds[5] - bounds[4]);
			double extent = diagonal.dot(axis);

			// Map cursor pos to position along axis
			double delta = (lastY - event.y) / 
				static_cast<double>(viewportHeight);
			Vector3 offset = extent * axis * delta;

			// Move plane origin
			renderer->SetPlane(origin + offset, right, up);

			handled = true;
		}

		lastX = event.x;
		lastY = event.y;

		if (!handled) handled = Superclass::MouseMoveEvent(event);
		return handled;
	}
}
