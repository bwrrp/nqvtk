#pragma once

namespace NQVTK
{
	class MouseEvent
	{
	public:
		enum MouseButtons
		{
			LeftButton   = 0x01,
			RightButton  = 0x02,
			MiddleButton = 0x04
		};
		// Button state
		int buttons;
		// Button that triggered the event (or 0)
		int button;
		// Mouse position
		int x;
		int y;
		// Modifiers
		bool control;
		bool shift;
		bool alt;
	};

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
