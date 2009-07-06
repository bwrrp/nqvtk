#include "BrushingInteractor.h"

#include "Rendering/BrushingRenderer.h"

#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	BrushingInteractor::BrushingInteractor(NQVTK::BrushingRenderer *renderer) 
	{
		assert(renderer);
		this->renderer = renderer;
	}

	// ------------------------------------------------------------------------
	bool BrushingInteractor::MouseMoveEvent(MouseEvent event)
	{
		int pen = 0;
		if (event.buttons & MouseEvent::LeftButton) pen = 1;
		if (event.buttons & MouseEvent::RightButton) pen = 2;
		renderer->LineTo(event.x, renderer->GetHeight() - event.y, pen);

		return true;
	}
}
