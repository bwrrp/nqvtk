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
}
