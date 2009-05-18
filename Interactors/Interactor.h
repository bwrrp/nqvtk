#pragma once

#include "MouseEvent.h"

namespace NQVTK
{
	class Interactor
	{
	public:
		Interactor();

		virtual bool MouseMoveEvent(MouseEvent event);
		virtual bool MousePressEvent(MouseEvent event);
		virtual bool MouseReleaseEvent(MouseEvent event);
		virtual void ResizeEvent(int width, int height);

	protected:
		int viewportWidth;
		int viewportHeight;

	private:
		// Not implemented
		Interactor(const Interactor&);
		void operator=(const Interactor&);
	};
}
