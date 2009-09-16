#pragma once

#include "Interactor.h"

namespace NQVTK
{
	class BrushingRenderer;

	class BrushingInteractor : public Interactor
	{
	public:
		typedef Interactor Superclass;

		BrushingInteractor(NQVTK::BrushingRenderer *renderer);

		virtual bool MouseMoveEvent(MouseEvent event);

	protected:
		NQVTK::BrushingRenderer *renderer;

	private:
		// Not implemented
		BrushingInteractor(const BrushingInteractor&);
		void operator=(const BrushingInteractor&);
	};
}
