#pragma once

#include "CrossEyedStereoRenderer.h"

#include "Camera.h"
#include "Renderables/Renderable.h"

#include "GLBlaat/GLFramebuffer.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	CrossEyedStereoRenderer::CrossEyedStereoRenderer()
	{
		leftEye = true;
		eyeSpacing = 0.1;
	}

	// ------------------------------------------------------------------------
	CrossEyedStereoRenderer::~CrossEyedStereoRenderer()
	{
	}

	// ------------------------------------------------------------------------
	void CrossEyedStereoRenderer::Resize(int w, int h)
	{
		// We render at half width
		Superclass::Resize(w / 2, h);
	}

	// ------------------------------------------------------------------------
	void CrossEyedStereoRenderer::Clear()
	{
		// Only clear for the left eye and when rendering to FBO
		if (leftEye || fbo1->IsBound()) Superclass::Clear();
	}

	// ------------------------------------------------------------------------
	void CrossEyedStereoRenderer::DrawCamera()
	{
		Vector3 offset = Vector3(0.0, 0.0, 1.0);
		if (leftEye)
		{
			offset.x = -eyeSpacing / 2.0;
		}
		else
		{
			offset.x = eyeSpacing / 2.0;
		}

		// TODO: fix me for new camera
		Renderable *renderable = renderables[0];
		camera->position = renderable->GetCenter() - offset;
		camera->FocusOn(renderable);

		// Set up the camera (matrices)
		camera->Draw();
	}

	// ------------------------------------------------------------------------
	void CrossEyedStereoRenderer::Draw()
	{
		// Draw left eye (on the right)
		leftEye = true;
		// Offset viewport
		viewportX = viewportWidth;
		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
		// Draw
		Superclass::Draw();

		// Draw right eye
		leftEye = false;
		// Offset viewport
		viewportX = 0;
		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
		// Draw
		Superclass::Draw();
	}
}
