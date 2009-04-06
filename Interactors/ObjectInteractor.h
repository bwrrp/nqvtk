#pragma once

#include "Interactor.h"

#include "Renderables/Renderable.h"

namespace NQVTK
{
	class ObjectInteractor : public NQVTK::Interactor
	{
	public:
		typedef Interactor Superclass;

		ObjectInteractor(NQVTK::Renderable *obj) : Interactor()
		{
			assert(obj);
			renderable = obj;
			lastX = lastY = 0;
		}

		virtual bool MouseMoveEvent(MouseEvent event)
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
				NQVTK::Vector3 right = NQVTK::Vector3(1.0, 0.0, 0.0);
				NQVTK::Vector3 up = NQVTK::Vector3(0.0, 1.0, 0.0);
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

	protected:
		NQVTK::Renderable *renderable;

		// Previous mouse coordinates
		int lastX;
		int lastY;

	private:
		// Not implemented
		ObjectInteractor(const ObjectInteractor&);
		void operator=(const ObjectInteractor&);
	};
}
