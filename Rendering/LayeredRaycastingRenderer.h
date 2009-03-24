#pragma once

#include "LayeredRenderer.h"

#include "Styles/LayeredRaycaster.h"

namespace NQVTK
{
	class LayeredRaycastingRenderer : public NQVTK::LayeredRenderer
	{
	public:
		typedef NQVTK::LayeredRenderer Superclass;

		LayeredRaycastingRenderer() : fboG(0), raycaster(0)
		{
			layeredRaycasting = false;
		}

		virtual ~LayeredRaycastingRenderer()
		{
			delete fboG;
			delete raycaster;
		}

		virtual bool Initialize()
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
				raycaster = style->CreateRaycaster();
				if (!raycaster)
				{
					qDebug("Failed to build Raycaster!");
					// We can't really expect the style to work without its raycaster
					// Otherwise we could just set layeredRaycasting to false...
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

		virtual void Resize(int w, int h)
		{
			Superclass::Resize(w, h);

			if (layeredRaycasting)
			{
				// The geometry infobuffer (peeling pass) and the layer infobuffer 
				// (raycasting pass) have the same format. If raycasting doesn't 
				// encounter anything interesting, the result is a simple copy. 
				// This allows us to easily combine geometry and volume features.
				// NOTE: The extra buffer is always created / resized / deleted for now
				if (!fboG) 
				{
					fboG = style->CreateFBO(w, h);
					// this fbo does not need a depth buffer
					delete fboG->DetachRendertarget(GL_DEPTH_ATTACHMENT_EXT);
					fboG->Unbind();
				}
				else
				{
					if (!fboG->Resize(w, h)) qDebug("WARNING! fboG resize failed!");
				}
			}
		}

		virtual void DrawScribePass(int layer)
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
				GLRendertarget *depthbuffer = fbo1->DetachRendertarget(GL_DEPTH_ATTACHMENT_EXT);
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
				GLRendertarget *depthbuffer = fboG->DetachRendertarget(GL_DEPTH_ATTACHMENT_EXT);
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

		virtual void DrawRaycastPass(int layer)
		{
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);

			// Start raycaster program
			raycaster->Start();
			raycaster->SetUniform1i("layer", layer);
			raycaster->SetUniform1f("farPlane", camera->farZ);
			raycaster->SetUniform1f("nearPlane", camera->nearZ);
			raycaster->SetUniform1f("viewportX", static_cast<float>(viewportX));
			raycaster->SetUniform1f("viewportY", static_cast<float>(viewportY));
			raycaster->SetUniform3f("cameraPos", 
				camera->position.x, camera->position.y, camera->position.z);
			ApplyParamSetsArrays(raycaster);
			// TODO: figure out where to put the raycaster parameters
			// For now, just apply both sets of parameters and see what fits
			style->UpdateScribeParameters(raycaster);
			style->UpdatePainterParameters(raycaster);

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

	protected:
		GLFramebuffer *fboG;
		GLProgram *raycaster;

		// Whether the current style requires layered raycasting
		bool layeredRaycasting;
	};
}
