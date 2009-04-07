#pragma once

#include "ObjectInteractor.h"

#include "Renderables/Renderable.h"

#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	ObjectInteractor::ObjectInteractor(Renderable *obj)
	{
		assert(obj);
		renderable = obj;
		lastX = lastY = 0;
	}

	// ------------------------------------------------------------------------
	bool ObjectInteractor::MouseMoveEvent(MouseEvent event)
	{
		bool handled = false;

		if (event.buttons & MouseEvent::LeftButton)
		{
			// Rotate
			renderable->rotateY += event.x - lastX;
			renderable->rotateX -= event.y - lastY;

			if (renderable->rotateX > 80.0) 
			{
				renderable->rotateX = 80.0;
			}
			if (renderable->rotateX < -80.0) 
			{
				renderable->rotateX = -80.0;
			}

			handled = true;
		}

		if (event.buttons & MouseEvent::MiddleButton)
		{
			// TODO: make translation relative to window
			Vector3 right = Vector3(1.0, 0.0, 0.0);
			Vector3 up = Vector3(0.0, 1.0, 0.0);
			double factor = 0.6;
			// Translate
			renderable->position += 
				(lastX - event.x) * factor * right +
				(lastY - event.y) * factor * up;

			handled = true;
		}

		lastX = event.x;
		lastY = event.y;

		return handled;
	}
}
