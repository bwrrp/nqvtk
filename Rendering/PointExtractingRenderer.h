#pragma once

#include "GLBlaat/GL.h"

#include "LayeredRenderer.h"

#include "Styles/RenderStyle.h"

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLOcclusionQuery.h"

#include <cassert>

namespace NQVTK 
{
	class PointExtractingRenderer : public LayeredRenderer
	{
	public:
		typedef LayeredRenderer Superclass;

		PointExtractingRenderer() : LayeredRenderer()
		{
			drawBackground = false;
		}

		virtual ~PointExtractingRenderer() { }

		virtual bool Initialize()
		{
			if (!Superclass::Initialize()) return false;

			// TODO: make sure we have a target FBO?

			// TODO: initialize storage for extracted points

			return true;
		}

		virtual void DrawPainterPass(int layer)
		{
			// Clear the target
			Clear();

			// Draw the painter pass
			Superclass::DrawPainterPass();

			// TODO: get image from target and extract points
		}

	protected:
		// TODO: point storage (VTK PolyData?)

	private:
		// Not implemented
		PointExtractingRenderer(const PointExtractingRenderer&);
		void operator=(const PointExtractingRenderer&);
	};
}
