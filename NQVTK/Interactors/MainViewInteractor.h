#pragma once

#include "Interactor.h"

namespace NQVTK
{
	class Renderer;
	class OverlayRenderer;
	class CameraInteractor;
	class ObjectInteractor;
	class BrushingInteractor;
	class Scene;

	class MainViewInteractor : public Interactor
	{
	public:
		typedef Interactor Superclass;

		MainViewInteractor(Renderer *ren); 
		virtual ~MainViewInteractor();

		void SetScene(Scene *scene);

		virtual bool MouseMoveEvent(MouseEvent event);
		virtual bool MousePressEvent(MouseEvent event);
		virtual bool MouseReleaseEvent(MouseEvent event);
		virtual void ResizeEvent(int width, int height);

	protected:
		CameraInteractor *cameraInt;
		ObjectInteractor *objectInt;
		ObjectInteractor *clipperInt;
		BrushingInteractor *brushInt;

		OverlayRenderer *oren;

	private:
		// Not implemented
		MainViewInteractor(const MainViewInteractor&);
		void operator=(const MainViewInteractor&);
	};
}
