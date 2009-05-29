#include "SliceRenderer.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"

#include "View.h"

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
		// TODO: set up blending (and figure out how we want to do this)

		// TODO: set up shader, textures...

		Clear();

		if (view)
		{
			// Draw a single slice for each renderable
			glBegin(GL_QUADS);
			for (unsigned int objectId = 0; 
				objectId < view->GetNumberOfRenderables(); 
				++objectId)
			{
				// The shader should discard renderables without volumes and 
				// generate appropriate alpha values or discards for blending

				// Visibility implies that the renderable is not null
				if (view->GetVisibility(objectId))
				{
					Renderable *renderable = view->GetRenderable(objectId);
					PrepareForRenderable(objectId, renderable);

					glColor4d(
						renderable->color.x, 
						renderable->color.y, 
						renderable->color.z, 
						renderable->opacity);

					// Draw the full screen quad for this plane
					glTexCoord3dv(origin.V);
					glVertex3d(-1.0, -1.0, 0.0);
					glTexCoord3dv((origin + right).V);
					glVertex3d(1.0, -1.0, 0.0);
					glTexCoord3dv((origin + right + up).V);
					glVertex3d(1.0, 1.0, 0.0);
					glTexCoord3dv((origin + up).V);
					glVertex3d(-1.0, 1.0, 0.0);
				}
			}
			glEnd();
		}

		glPopAttrib();

		if (fboTarget)
		{
			fboTarget->Unbind();
		}
	}

	// ------------------------------------------------------------------------
	void SliceRenderer::SetPlane(const Vector3 &origin, const Vector3 &up, 
		const Vector3 &right)
	{
		this->origin = origin;
		this->up = up;
		this->right = right;
	}
}
