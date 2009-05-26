#pragma once

#include "CrossEyedStereoRenderer.h"

#include "Camera.h"
#include "Renderables/Renderable.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	CrossEyedStereoRenderer::CrossEyedStereoRenderer(Renderer *baseRenderer)
		: NestedRenderer(baseRenderer), fboEyeHelper(0)
	{
		assert(baseRenderer);
		eyeSpacing = 0.1;
	}

	// ------------------------------------------------------------------------
	CrossEyedStereoRenderer::~CrossEyedStereoRenderer()
	{
		delete fboEyeHelper;
	}

	// ------------------------------------------------------------------------
	bool CrossEyedStereoRenderer::Initialize()
	{
		bool ok = baseRenderer->Initialize();
		delete fboEyeHelper;
		fboEyeHelper = 0;

		return ok;
	}

	// ------------------------------------------------------------------------
	void CrossEyedStereoRenderer::SetViewport(int x, int y, int w, int h)
	{
		// We render at half width
		baseRenderer->SetViewport(x, y, w / 2, h);
		Superclass::SetViewport(x, y, w, h);

		// Create an FBO for one of the eyes
		if (!fboEyeHelper)
		{
			fboEyeHelper = GLFramebuffer::New(w / 2, h);
			assert(fboEyeHelper);
			fboEyeHelper->CreateColorTexture();
			fboEyeHelper->CreateDepthBuffer();
			if (!fboEyeHelper->IsOk())
			{
				std::cerr << "WARNING! fboEyeHelper not ok!" << std::endl;
			}
			fboEyeHelper->Unbind();
		}
		else
		{
			if (!fboEyeHelper->Resize(w / 2, h))
			{
				std::cerr << "WARNING! fboEyeHelper resize failed!" << 
					std::endl;
			}
		}
	}

	// ------------------------------------------------------------------------
	void CrossEyedStereoRenderer::Clear()
	{
		baseRenderer->Clear();
	}

	// ------------------------------------------------------------------------
	void CrossEyedStereoRenderer::Draw()
	{
		// Sync camera
		UpdateCamera();

		Vector3 forward = camera->focus - camera->position;
		Vector3 right = forward.cross(camera->up);
		Vector3 offset = 5 * eyeSpacing * right.normalized();

		// Draw left eye to the FBO
		baseRenderer->SetTarget(fboEyeHelper);

		// Set viewport for FBO
		baseRenderer->SetViewport(0, 0, viewportWidth / 2, viewportHeight);

		// Offset camera
		baseRenderer->GetCamera()->position = camera->position - offset;

		// Draw left eye image
		baseRenderer->Draw();

		// Draw right eye directly to this renderer's target
		baseRenderer->SetTarget(0);
		if (fboTarget)
		{
			fboTarget->Bind();
		}
		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

		// Offset viewport
		baseRenderer->SetViewport(viewportX, viewportY, 
			viewportWidth / 2, viewportHeight);

		// Offset camera
		baseRenderer->GetCamera()->position = camera->position + offset;

		// Draw
		baseRenderer->Draw();

		// Restore viewport
		SetViewport(viewportX, viewportY, viewportWidth, viewportHeight);

		// Draw left eye image into the target
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		DrawTexture(fboEyeHelper->GetTexture2D(), 0.0, 1.0);
		glPopAttrib();

		if (fboTarget)
		{
			fboTarget->Unbind();
		}

		// Restore viewport (again)
		SetViewport(viewportX, viewportY, viewportWidth, viewportHeight);
	}
}
