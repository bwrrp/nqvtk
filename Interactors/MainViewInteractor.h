#pragma once

#include "Interactor.h"

#include "CameraInteractor.h"
#include "OrbitCameraInteractor.h"
#include "ArcballCameraInteractor.h"
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

			// Create the proper camera interactor
			NQVTK::OrbitCamera *ocam = dynamic_cast<NQVTK::OrbitCamera*>(ren->GetCamera());
			if (ocam)
			{
				cameraInt = new NQVTK::OrbitCameraInteractor(ocam);
			}
			else
			{
				NQVTK::ArcballCamera *acam = dynamic_cast<NQVTK::ArcballCamera*>(ren->GetCamera());
				if (acam) cameraInt = new NQVTK::ArcballCameraInteractor(acam);
			}
			// Create object interactors
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
				return Superclass::MouseMoveEvent(event);
			}
		}

		virtual bool MousePressEvent(QMouseEvent *event)
		{
			// Only the camera interactor uses this for now
			if (cameraInt) return cameraInt->MousePressEvent(event);
			return Superclass::MousePressEvent(event);
		}

		virtual bool MouseReleaseEvent(QMouseEvent *event)
		{
			// Only the camera interactor uses this for now
			if (cameraInt) return cameraInt->MouseReleaseEvent(event);
			return Superclass::MouseReleaseEvent(event);
		}

		virtual void ResizeEvent(int width, int height)
		{
			if (cameraInt) cameraInt->ResizeEvent(width, height);
			if (objectInt) objectInt->ResizeEvent(width, height);
			if (clipperInt) clipperInt->ResizeEvent(width, height);
			if (brushInt) brushInt->ResizeEvent(width, height);
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
