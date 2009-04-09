#pragma once

#include "NestedRenderer.h"

#include "Camera.h"

#include "GLBlaat/GLTexture.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	NestedRenderer::NestedRenderer(Renderer *baseRenderer)
		: baseRenderer(baseRenderer)
	{
	}

	// ------------------------------------------------------------------------
	NestedRenderer::~NestedRenderer()
	{
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
}
