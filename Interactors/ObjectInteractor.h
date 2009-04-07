#pragma once

#include "Interactor.h"

namespace NQVTK
{
	class Renderable;

	class ObjectInteractor : public Interactor
	{
	public:
		typedef Interactor Superclass;

		ObjectInteractor(Renderable *obj);

		virtual bool MouseMoveEvent(MouseEvent event);

	protected:
		Renderable *renderable;

		// Previous mouse coordinates
		int lastX;
		int lastY;

	private:
		// Not implemented
		ObjectInteractor(const ObjectInteractor&);
		void operator=(const ObjectInteractor&);
	};
}
