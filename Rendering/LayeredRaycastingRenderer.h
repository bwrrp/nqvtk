#pragma once

#include "LayeredRenderer.h"

namespace NQVTK
{
	class LayeredRaycastingRenderer : public NQVTK::LayeredRenderer
	{
	public:
		typedef NQVTK::LayeredRenderer Superclass;

		LayeredRaycastingRenderer() : fboG(0)
		{
		}

		virtual ~LayeredRaycastingRenderer()
		{
			delete fboG;
		}

		virtual void Resize(int w, int h)
		{
			Superclass::Resize(w, h);

			// TODO: how to deal with the shaders/styles for the extra pass?
			if (!fboG) 
			{
				fboG = style->CreateFBO(w, h);
			}
			else
			{
				if (!fboG->Resize(w, h)) qDebug("WARNING! fboG resize failed!");
			}
		}

		virtual void DrawScribePass(int layer)
		{
			// Bind the geometry layer FBO
			fbo1->Unbind();
			fboG->Bind();

			// Draw the scribe pass to render the next geometry layer infobuffer
			// TODO: This assumes the style's scribe is the peel scribe pass
			Superclass::DrawScribePass(layer);

			// Restore the normal scribe stage FBO
			fboG->Unbind();
			fbo1->Bind();

			// TODO: Draw the raycasting scribe pass to render the actual infobuffer
		}

	protected:
		GLFramebuffer *fboG;
	};
}
