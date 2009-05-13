#pragma once

#include "LayeredRaycastingRenderer.h"

#include "Camera.h"
#include "Styles/LayeredRaycaster.h"

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTextureManager.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	LayeredRaycastingRenderer::LayeredRaycastingRenderer() 
		: fboG(0), raycaster(0)
	{
		layeredRaycasting = false;
	}

	// ------------------------------------------------------------------------
	LayeredRaycastingRenderer::~LayeredRaycastingRenderer()
	{
		delete fboG;
		delete raycaster;
	}

	// ------------------------------------------------------------------------
	bool LayeredRaycastingRenderer::Initialize()
	{
		if (!Superclass::Initialize()) return false;

		delete fboG;
		fboG = 0;

		delete raycaster;
		raycaster = 0;

		NQVTK::Styles::LayeredRaycaster *style = 
			dynamic_cast<NQVTK::Styles::LayeredRaycaster*>(this->style);
		if (style)
		{
			std::cout << "Creating Raycaster..." << std::endl;
			raycaster = style->CreateRaycaster();
			if (!raycaster)
			{
				std::cerr << "Failed to build Raycaster!" << std::endl;
				// We can't really expect the style to work without its 
				// raycaster, or we could set layeredRaycasting to false...
				return false;
			}

			// Ready for layered raycasting
			layeredRaycasting = true;
		}
		else
		{
			layeredRaycasting = false;
		}

		return true;
	}

	// ------------------------------------------------------------------------
	void LayeredRaycastingRenderer::SetViewport(int x, int y, int w, int h)
	{
		Superclass::SetViewport(x, y, w, h);

		if (layeredRaycasting)
		{
			// The geometry infobuffer (peeling pass) and the layer infobuffer 
			// (raycasting pass) have the same format. If raycasting doesn't 
			// encounter anything interesting, the result is a simple copy. 
			// This allows us to easily combine geometry and volume features.
			// NOTE: The extra buffer is always created/resized/deleted for now
			if (!fboG) 
			{
				fboG = style->CreateFBO(w, h);
				// this fbo does not need a depth buffer
				delete fboG->DetachRendertarget(GL_DEPTH_ATTACHMENT_EXT);
				fboG->Unbind();
			}
			else
			{
				if (!fboG->Resize(w, h)) 
				{
					std::cerr << "WARNING! fboG resize failed!" << std::endl;
				}
			}
		}
	}

	// ------------------------------------------------------------------------
	void LayeredRaycastingRenderer::DrawScribePass(int layer)
	{
		Clear();

		// The first layer is a special case (only geometry, no volumes)
		// For all other layers, we split the scribe stage into two passes:
		// - Peeling - renders geometry (bounding and other) with depth peeling
		// - Raycasting - renders volume features
		// Both result in an infobuffer, which is visualized by the painter.
		if (layer > 0 && layeredRaycasting)
		{
			// Bind the geometry layer FBO
			// We need to use the same depth buffer as fbo1 here
			GLRendertarget *depthbuffer = fbo1->DetachRendertarget(
				GL_DEPTH_ATTACHMENT_EXT);
			fbo1->Unbind();
			fboG->Bind();
			fboG->AttachRendertarget(GL_DEPTH_ATTACHMENT_EXT, depthbuffer);
			assert(fboG->IsOk());
		}

		// Draw the scribe pass to render the next geometry layer infobuffer
		// TODO: This assumes scribe is the peel scribe pass
		Superclass::DrawScribePass(layer);

		if (layer > 0 && layeredRaycasting)
		{
			// Restore the normal scribe stage FBO
			GLRendertarget *depthbuffer = fboG->DetachRendertarget(
				GL_DEPTH_ATTACHMENT_EXT);
			fboG->Unbind();
			fbo1->Bind();
			fbo1->AttachRendertarget(GL_DEPTH_ATTACHMENT_EXT, depthbuffer);
			assert(fbo1->IsOk());

			// Draw the raycasting scribe pass to render the actual infobuffer
			// The raycasting pass is more similar to the painter pass in that 
			// no geometry is rendered (full screen pass). Additionally, both 
			// the previous layer and the result of the peel pass are used as 
			// input textures. The output, however, is an info buffer.
			DrawRaycastPass(layer);
		}
	}

	// ------------------------------------------------------------------------
	void LayeredRaycastingRenderer::DrawRaycastPass(int layer)
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		// Start raycaster program
		raycaster->Start();
		raycaster->SetUniform1i("layer", layer);
		raycaster->SetUniform1f("farPlane", static_cast<float>(camera->farZ));
		raycaster->SetUniform1f("nearPlane", static_cast<float>(camera->nearZ));
		// The raycast pass is drawn to a separate framebuffer, so the 
		// viewport starts at (0, 0) for this pass
		raycaster->SetUniform1f("viewportX", 0.0f);
		raycaster->SetUniform1f("viewportY", 0.0f);
		raycaster->SetUniform3f("cameraPos", 
			static_cast<float>(camera->position.x), 
			static_cast<float>(camera->position.y), 
			static_cast<float>(camera->position.z));
		
		// TODO: figure out where to put the raycaster parameters
		// For now, just apply both sets of parameters and see what fits
		style->UpdateScribeParameters(raycaster);
		style->UpdatePainterParameters(raycaster);

		ApplyParamSetsArrays(raycaster);

		// The raycaster uses the same input as the painter, except that the 
		// "current" layer is the geometry layer from the peeling pass
		style->RegisterPainterTextures(fboG, fbo2);
		tm->SetupProgram(raycaster);
		tm->Bind();

		// Draw a full screen quad for the raycasting pass
		glColor3d(1.0, 1.0, 1.0);
		glBegin(GL_QUADS);
		glVertex3d(-1.0, -1.0, 0.0);
		glVertex3d(1.0, -1.0, 0.0);
		glVertex3d(1.0, 1.0, 0.0);
		glVertex3d(-1.0, 1.0, 0.0);
		glEnd();

		raycaster->Stop();

		// Clean up references in the texture manager
		tm->Unbind();
		style->UnregisterPainterTextures();

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}
}
