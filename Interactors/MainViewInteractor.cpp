#pragma once

#include "MainViewInteractor.h"

#include "CameraInteractor.h"
#include "OrbitCameraInteractor.h"
#include "ArcballCameraInteractor.h"
#include "ObjectInteractor.h"
#include "BrushingInteractor.h"

#include "Rendering/ArcballCamera.h"
#include "Rendering/OrbitCamera.h"

#include "Rendering/Scene.h"

#include "Rendering/Renderer.h"
#include "Rendering/OverlayRenderer.h"
#include "Rendering/BrushingRenderer.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	MainViewInteractor::MainViewInteractor(Renderer *ren, Scene *scene) 
		: Interactor(), cameraInt(0), objectInt(0), clipperInt(0), brushInt(0)
	{
		// Create the proper camera interactor
		OrbitCamera *ocam = dynamic_cast<OrbitCamera*>(
			ren->GetCamera());
		if (ocam)
		{
			cameraInt = new OrbitCameraInteractor(ocam);
		}
		else
		{
			NQVTK::ArcballCamera *acam = dynamic_cast<ArcballCamera*>(
				ren->GetCamera());
			if (acam) cameraInt = new ArcballCameraInteractor(acam);
		}
		// Is it an overlay renderer?
		oren = dynamic_cast<OverlayRenderer*>(ren);
		if (oren)
		{
			// TODO: simply switch the interactor for brushing
			// This is a bit of a hack to test everything
			NQVTK::BrushingRenderer *bren = 
				dynamic_cast<BrushingRenderer*>(oren->GetOverlayRenderer());
			if (bren) brushInt = new BrushingInteractor(bren);
			ren = oren->GetBaseRenderer();
		}
		// Create object interactors
		if (scene)
		{
			NQVTK::Renderable *renderable = scene->GetRenderable(0);
			if (renderable) objectInt = new ObjectInteractor(renderable);
			// TODO: should use clipperId from the ibisStyle
			NQVTK::Renderable *clipper = scene->GetRenderable(2);
			if (clipper) clipperInt = new ObjectInteractor(clipper);
		}
	}

	// ------------------------------------------------------------------------
	MainViewInteractor::~MainViewInteractor()
	{
		delete cameraInt;
		delete objectInt;
		delete clipperInt;
	}

	// ------------------------------------------------------------------------
	bool MainViewInteractor::MouseMoveEvent(MouseEvent event)
	{
		if (oren) oren->updateBase = true;

		if (event.control && objectInt)
		{
			// Control controls renderable 0
			return objectInt->MouseMoveEvent(event);
		}
		else if (event.alt && clipperInt)
		{
			// Alt controls the clipper (renderable 2)
			return clipperInt->MouseMoveEvent(event);
		}
		else if (event.shift && brushInt)
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

	// ------------------------------------------------------------------------
	bool MainViewInteractor::MousePressEvent(MouseEvent event)
	{
		// Only the camera interactor uses this for now
		if (cameraInt) return cameraInt->MousePressEvent(event);
		return Superclass::MousePressEvent(event);
	}

	// ------------------------------------------------------------------------
	bool MainViewInteractor::MouseReleaseEvent(MouseEvent event)
	{
		// Only the camera interactor uses this for now
		if (cameraInt) return cameraInt->MouseReleaseEvent(event);
		return Superclass::MouseReleaseEvent(event);
	}

	// ------------------------------------------------------------------------
	void MainViewInteractor::ResizeEvent(int width, int height)
	{
		if (cameraInt) cameraInt->ResizeEvent(width, height);
		if (objectInt) objectInt->ResizeEvent(width, height);
		if (clipperInt) clipperInt->ResizeEvent(width, height);
		if (brushInt) brushInt->ResizeEvent(width, height);
	}
}
