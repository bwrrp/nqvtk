#pragma once

#include <QMouseEvent>

namespace NQVTK
{
	class Interactor
	{
	public:
		Interactor() { }

		virtual bool MouseMoveEvent(QMouseEvent *event)
		{
			return false;
		}

		virtual bool MousePressEvent(QMouseEvent *event)
		{
			return false;
		}

		virtual bool MouseReleaseEvent(QMouseEvent *event)
		{
			return false;
		}

		virtual void ResizeEvent(int width, int height) 
		{
			viewportWidth = width;
			viewportHeight = height;
		}

	protected:
		int viewportWidth;
		int viewportHeight;

	private:
		// Not implemented
		Interactor(const Interactor&);
		void operator=(const Interactor&);
	};
}
