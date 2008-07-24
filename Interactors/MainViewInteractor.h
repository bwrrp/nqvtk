#pragma once

#include "Interactor.h"

#include "CameraInteractor.h"
#include "ObjectInteractor.h"

#include "Rendering/Renderer.h"

namespace NQVTK
{
	class MainViewInteractor : public NQVTK::Interactor
	{
	public:
		typedef Interactor Superclass;

		MainViewInteractor(NQVTK::Renderer *ren) 
			: Interactor(), cameraInt(0), objectInt(0), clipperInt(0)
		{
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

	private:
		// Not implemented
		MainViewInteractor(const MainViewInteractor&);
		void operator=(const MainViewInteractor&);
	};
}
