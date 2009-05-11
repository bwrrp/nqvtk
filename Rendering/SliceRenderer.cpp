#include "SliceRenderer.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"

#include "Renderables/Renderable.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	SliceRenderer::SliceRenderer()
	{
	}

	// ------------------------------------------------------------------------
	SliceRenderer::~SliceRenderer()
	{
	}

	// ------------------------------------------------------------------------
	void SliceRenderer::PrepareForRenderable(int objectId, 
		Renderable *renderable)
	{
		/*
		if (shader)
		{
			if (shaderAttribs.size() > 0)
			{
				NQVTK::VBOMesh *mesh = dynamic_cast<NQVTK::VBOMesh*>(renderable);
				if (mesh) mesh->SetupAttributes(shaderAttribs);
			}
			renderable->ApplyParamSets(shader);
		}
		*/
	}

	// ------------------------------------------------------------------------
	void SliceRenderer::Draw()
	{
		// Prepare for rendering
		if (fboTarget)
		{
			fboTarget->Bind();
		}
		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		// TODO: set up blending

		// TODO: set up shader, textures...
		// TODO: It would be nice if the paramsets could handle per-object textures now
		// This requires them to register with the tm before rendering and swap in 
		// their textures during PrepareForRenderable (in addition to setting uniforms)

		Clear();

		// Draw a single slice for each renderable
		for (std::vector<Renderable*>::const_iterator it = renderables.begin();
			it != renderables.end(); ++it)
		{
			// NOTE: The shader should discard renderables without volumes
			// It's up to the shader to generate alpha values or discards
		}

		glPopAttrib();

		if (fboTarget)
		{
			fboTarget->Unbind();
		}
	}
}
