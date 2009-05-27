#pragma once

#include "NestedRenderer.h"

#include "Camera.h"
#include "View.h"

#include "GLBlaat/GLTexture.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	NestedRenderer::NestedRenderer(Renderer *baseRenderer)
		: baseRenderer(baseRenderer)
	{
		// We share the view of the base Renderer
		view = baseRenderer->GetView();
	}

	// ------------------------------------------------------------------------
	NestedRenderer::~NestedRenderer()
	{
		// The view is shared, so prevent double deletion
		view = 0;
		delete baseRenderer;
	}

	// ------------------------------------------------------------------------
	void NestedRenderer::SetViewport(int x, int y, int w, int h)
	{
		Superclass::SetViewport(x, y, w, h);
		UpdateCamera();
		// NOTE: Setting baseRenderer's viewport is up to the subclass
		// because it may not be the same as for the outer renderer
	}

	// ------------------------------------------------------------------------
	void NestedRenderer::DrawCamera()
	{
		UpdateCamera();
		baseRenderer->DrawCamera();
	}

	// ------------------------------------------------------------------------
	void NestedRenderer::UpdateCamera()
	{
		// The call to GetCamera ensures we have a camera
		baseRenderer->GetCamera()->position = GetCamera()->position;
		baseRenderer->GetCamera()->focus = camera->focus;
		baseRenderer->GetCamera()->up = camera->up;
	}

	// ------------------------------------------------------------------------
	void NestedRenderer::ResetTextures()
	{
		Superclass::ResetTextures();
		baseRenderer->ResetTextures();
	}

	// ------------------------------------------------------------------------
	void NestedRenderer::SetView(NQVTK::View *view)
	{
		// The view is shared for easy access
		baseRenderer->SetView(view);
		this->view = 0;
		Superclass::SetView(view);
	}
}
