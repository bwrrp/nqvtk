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

			NQVTK::Styles::LayeredRaycaster *style = 
				dynamic_cast<NQVTK::Styles::LayeredRaycaster*>(this->style);
			if (style)
			{
				// TODO: get raycaster program from style instead
				raycaster = style->CreateRaycaster();
				if (!raycaster)
				{
					qDebug("Failed to build Raycaster!");
					// We can't really expect the style to work without it's raycaster
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
				}
				else
				{
					if (!fboG->Resize(w, h)) qDebug("WARNING! fboG resize failed!");
				}
			}
		}

		virtual void DrawScribePass(int layer)
		{
			// The first layer is a special case (only geometry, no volumes)
			// For all other layers, we split the scribe stage into two passes:
			// - Peeling - renders geometry (bounding and other) with depth peeling
			// - Raycasting - renders volume features
			// Both result in an infobuffer, which is visualized by the painter.
			if (layer > 0 && layeredRaycasting)
			{
				// Bind the geometry layer FBO
				fbo1->Unbind();
				fboG->Bind();
			}

			// Draw the scribe pass to render the next geometry layer infobuffer
			// TODO: This assumes scribe is the peel scribe pass
			Superclass::DrawScribePass(layer);

			if (layer > 0 && layeredRaycasting)
			{
				// Restore the normal scribe stage FBO
				fboG->Unbind();
				fbo1->Bind();

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
			// We assume parameters are shared between both scribe passes
			style->UpdateScribeParameters(raycaster);

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
		}

	protected:
		GLFramebuffer *fboG;
		GLProgram *raycaster;

		// Whether the current style requires layered raycasting
		bool layeredRaycasting;
	};
}
