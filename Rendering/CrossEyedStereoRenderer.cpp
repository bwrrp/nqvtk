#pragma once

#include "CrossEyedStereoRenderer.h"

#include "Camera.h"
#include "Renderables/Renderable.h"

#include "GLBlaat/GLFramebuffer.h"

#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	CrossEyedStereoRenderer::CrossEyedStereoRenderer(Renderer *baseRenderer)
		: NestedRenderer(baseRenderer)
	{
		assert(baseRenderer);
		eyeSpacing = 0.1;
	}

	// ------------------------------------------------------------------------
	CrossEyedStereoRenderer::~CrossEyedStereoRenderer()
	{
		delete baseRenderer;
	}

	// ------------------------------------------------------------------------
	bool CrossEyedStereoRenderer::Initialize()
	{
		return baseRenderer->Initialize();
	}

	// ------------------------------------------------------------------------
	void CrossEyedStereoRenderer::SetViewport(int x, int y, int w, int h)
	{
		// We render at half width
		baseRenderer->SetViewport(x, y, w / 2, h);
		Superclass::SetViewport(x, y, w, h);
	}

	// ------------------------------------------------------------------------
	void CrossEyedStereoRenderer::Clear()
	{
		baseRenderer->Clear();
	}

	// ------------------------------------------------------------------------
	void CrossEyedStereoRenderer::Draw()
	{
		if (fboTarget)
		{
			fboTarget->Bind();
		}
		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

		// Sync camera
		UpdateCamera();

		Vector3 forward = camera->focus - camera->position;
		Vector3 right = forward.cross(camera->up);
		Vector3 offset = 0.5 * eyeSpacing * right.normalized();

		// Draw left eye (on the right)

		// Offset viewport
		baseRenderer->SetViewport(viewportX + viewportWidth / 2, viewportY, 
			viewportWidth / 2, viewportHeight);

		// Offset camera
		baseRenderer->GetCamera()->position = camera->position - offset;

		// Draw
		baseRenderer->Draw();

		// Draw right eye

		// Offset viewport
		baseRenderer->SetViewport(viewportX, viewportY, 
			viewportWidth / 2, viewportHeight);

		// Offset camera
		baseRenderer->GetCamera()->position = camera->position + offset;

		// Draw
		baseRenderer->Draw();

		if (fboTarget)
		{
			fboTarget->Unbind();
		}

		// Restore viewport
		SetViewport(viewportX, viewportY, viewportWidth, viewportHeight);
	}
}
