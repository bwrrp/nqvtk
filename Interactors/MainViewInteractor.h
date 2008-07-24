#pragma once

#include "Interactor.h"

#include "CameraInteractor.h"
#include "ObjectInteractor.h"
#include "BrushingInteractor.h"

#include "Rendering/Renderer.h"
#include "Rendering/OverlayRenderer.h"
#include "Rendering/BrushingRenderer.h"

namespace NQVTK
{
	class MainViewInteractor : public NQVTK::Interactor
	{
	public:
		typedef Interactor Superclass;

		MainViewInteractor(NQVTK::Renderer *ren) 
			: Interactor(), cameraInt(0), objectInt(0), clipperInt(0), brushInt(0)
		{
			// Is it an overlay renderer?
			oren = dynamic_cast<NQVTK::OverlayRenderer*>(ren);
			if (oren)
			{
				// TODO: we probably want to simply switch the interactor while brushing
				// This is a bit of a hack to test everything
				NQVTK::BrushingRenderer *bren = 
					dynamic_cast<NQVTK::BrushingRenderer*>(oren->GetOverlayRenderer());
				if (bren) brushInt = new NQVTK::BrushingInteractor(bren);
				ren = oren->GetBaseRenderer();
			}

			NQVTK::OrbitCamera *ocam = dynamic_cast<NQVTK::OrbitCamera*>(ren->GetCamera());
			if (ocam) cameraInt = new NQVTK::CameraInteractor(ocam);
			NQVTK::Renderable *renderable = ren->GetRenderable(0);
			if (renderable) objectInt = new NQVTK::ObjectInteractor(renderable);
			NQVTK::Renderable *clipper = ren->GetRenderable(2);
			if (clipper) clipperInt = new NQVTK::ObjectInteractor(clipper);
		}

		virtual ~MainViewInteractor()
		{
			if (cameraInt) delete cameraInt;
			if (objectInt) delete objectInt;
			if (clipperInt) delete clipperInt;
		}

		// TODO: we might want to make this independent of Qt some day
		virtual bool MouseMoveEvent(QMouseEvent *event)
		{
			if (oren) oren->updateBase = true;

			if (event->modifiers() & Qt::ControlModifier && objectInt)
			{
				// Control controls renderable 0
				return objectInt->MouseMoveEvent(event);
			}
			else if (event->modifiers() & Qt::AltModifier && clipperInt)
			{
				// Alt controls the clipper (renderable 2)
				return clipperInt->MouseMoveEvent(event);
			}
			else if (event->modifiers() & Qt::ShiftModifier && brushInt)
			{
				// HACK: Shift controls brushing
				if (oren) oren->updateBase = false;
				return brushInt->MouseMoveEvent(event);
			}
			else if (cameraInt)
			{
				// No modifiers: camera control
				return cameraInt->MouseMoveEvent(event);
			}
			else
			{
				return false;
			}
		}

	protected:
		NQVTK::CameraInteractor *cameraInt;
		NQVTK::ObjectInteractor *objectInt;
		NQVTK::ObjectInteractor *clipperInt;
		NQVTK::BrushingInteractor *brushInt;

		NQVTK::OverlayRenderer *oren;

	private:
		// Not implemented
		MainViewInteractor(const MainViewInteractor&);
		void operator=(const MainViewInteractor&);
	};
}
