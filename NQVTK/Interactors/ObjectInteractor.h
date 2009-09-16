#pragma once

#include "Interactor.h"

namespace NQVTK
{
	class Scene;

	class ObjectInteractor : public Interactor
	{
	public:
		typedef Interactor Superclass;

		ObjectInteractor(Scene *obj, unsigned int objectId);

		virtual bool MouseMoveEvent(MouseEvent event);

	protected:
		Scene *scene;
		unsigned int objectId;

		// Previous mouse coordinates
		int lastX;
		int lastY;

	private:
		// Not implemented
		ObjectInteractor(const ObjectInteractor&);
		void operator=(const ObjectInteractor&);
	};
}
