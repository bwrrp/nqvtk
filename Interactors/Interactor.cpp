#pragma once

#include "Interactor.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	Interactor::Interactor()
	{
	}

	// ------------------------------------------------------------------------
	bool Interactor::MouseMoveEvent(MouseEvent event)
	{
		return false;
	}

	// ------------------------------------------------------------------------
	bool Interactor::MousePressEvent(MouseEvent event)
	{
		return false;
	}

	// ------------------------------------------------------------------------
	bool Interactor::MouseReleaseEvent(MouseEvent event)
	{
		return false;
	}

	// ------------------------------------------------------------------------
	void Interactor::ResizeEvent(int width, int height) 
	{
		viewportWidth = width;
		viewportHeight = height;
	}
}
