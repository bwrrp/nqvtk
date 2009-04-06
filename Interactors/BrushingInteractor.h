#pragma once

#include "Interactor.h"

#include "Rendering/BrushingRenderer.h"

namespace NQVTK
{
	class BrushingInteractor : public NQVTK::Interactor
	{
	public:
		typedef Interactor Superclass;

		BrushingInteractor(NQVTK::BrushingRenderer *renderer) : Interactor()
		{
			assert(renderer);
			this->renderer = renderer;
		}

		virtual bool MouseMoveEvent(MouseEvent event)
		{
			int pen = 0;
			if (event.buttons & MouseEvent::LeftButton) pen = 1;
			if (event.buttons & MouseEvent::RightButton) pen = 2;
			renderer->LineTo(event.x, renderer->GetHeight() - event.y, pen);

			return true;
		}

	protected:
		NQVTK::BrushingRenderer *renderer;

	private:
		// Not implemented
		BrushingInteractor(const BrushingInteractor&);
		void operator=(const BrushingInteractor&);
	};
}
