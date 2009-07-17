#pragma once

#include "Interactor.h"

namespace NQVTK
{
	class SliceRenderer;

	class SliceViewInteractor : public Interactor
	{
	public:
		typedef Interactor Superclass;

		SliceViewInteractor(SliceRenderer *renderer);

		virtual bool MouseMoveEvent(MouseEvent event);

	protected:
		int lastX;
		int lastY;

		SliceRenderer *renderer;

	private:
		// Not implemented
		SliceViewInteractor(const SliceViewInteractor&);
		void operator=(const SliceViewInteractor&);
	};
}
