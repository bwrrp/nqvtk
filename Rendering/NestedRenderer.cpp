#pragma once

#include "NestedRenderer.h"

#include "Camera.h"

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
