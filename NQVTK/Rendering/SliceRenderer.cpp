#include "SliceRenderer.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTextureManager.h"

#include "View.h"

#include "Renderables/Renderable.h"
#include "Renderables/VBOMesh.h"

namespace NQVTK
{
	// ------------------------------------------------------------------------
	SliceRenderer::SliceRenderer()
		: shader(0)
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
		if (shader) shader->SetUniform1i("objectId", objectId);
		if (shaderAttribs.size() > 0)
		{
			renderable->SetupAttributes(shaderAttribs);
		}
		// If shader is null, this will still setup textures
		// (remember to enable them during rendering...)
		renderable->ApplyParamSets(shader, tm);
		tm->Bind();
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

		// TODO: per-object textures are not initialized before SetupProgram
		// This doesn't matter in layered rendering because the scribe doesn't 
		// use them, but will add them to the tm. However, here we don't have 
		// such an extra pass...
		if (view)
		{
			if (shader) shader->Start();
			for (unsigned int objectId = 0; 
				objectId < view->GetNumberOfRenderables(); 
				++objectId)
			{
				Renderable *renderable = view->GetRenderable(objectId);
				PrepareForRenderable(objectId, renderable);
			}
			if (shader) shader->Stop();
		}

		Clear();

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		// Use alpha blending for now (with premultiplied alpha)
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		if (view)
		{
			if (shader)
			{
				shader->Start();
				tm->SetupProgram(shader);
				tm->Bind();
			}

			// Compute origin and right/up vectors which preserve aspect ratio
			UpdateViewportPlane();

			// Draw a single slice for each renderable
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

					// Draw the full screen quad for our slice plane
					glBegin(GL_QUADS);
					glTexCoord3dv(vporigin.V);
					glVertex3d(-1.0, -1.0, 0.0);
					glTexCoord3dv((vporigin + vpright).V);
					glVertex3d(1.0, -1.0, 0.0);
					glTexCoord3dv((vporigin + vpright + vpup).V);
					glVertex3d(1.0, 1.0, 0.0);
					glTexCoord3dv((vporigin + vpup).V);
					glVertex3d(-1.0, 1.0, 0.0);
					glEnd();
				}
			}

			if (shader)
			{
				tm->Unbind();
				shader->Stop();
			}
		}

		glPopAttrib();

		if (fboTarget)
		{
			fboTarget->Unbind();
		}
	}

	// ------------------------------------------------------------------------
	void SliceRenderer::SetPlane(const Vector3 &origin, 
		const Vector3 &right, const Vector3 &up)
	{
		this->origin = origin;
		this->right = right;
		this->up = up;
	}

	// ------------------------------------------------------------------------
	GLProgram *SliceRenderer::SetShader(GLProgram *shader)
	{
		GLProgram *oldshader = this->shader;
		this->shader = shader;
		shaderAttribs.clear();
		if (shader) shaderAttribs = shader->GetActiveAttributes();
		if (shader == oldshader) return 0;
		return oldshader;
	}

	// ------------------------------------------------------------------------
	bool SliceRenderer::CreateDefaultShader()
	{
		GLProgram *shader = GLProgram::New();
		if (!shader) return false;
		bool ok = true;
		if (ok) ok = shader->AddVertexShader(
			"void main() {"
			"   gl_TexCoord[0] = gl_MultiTexCoord0;"
			"   gl_Position = gl_Vertex;"
			"}");
		// TODO: we need the actual object transform here
		if (ok) ok = shader->AddFragmentShader(
			"uniform sampler3D volume;"
			"uniform vec3 volumeOrigin;"
			"uniform vec3 volumeSize;"
			"uniform float volumeDataShift;"
			"uniform float volumeDataScale;"
			"uniform int objectId;"
			"void main() {"
			"   vec3 tc = (gl_TexCoord[0].xyz - volumeOrigin) / volumeSize;"
			"   vec3 tex = /*vec3(volumeDataShift) + volumeDataScale * */"
			"      texture3D(volume, tc).xyz;"
			"   vec4 color = vec4(tex, length(tex));"
			"   for (int i = 0; i < 3; ++i) {"
			"      if (tc[i] < 0.0 || tc[i] > 1.0) {"
			"         color = vec4(vec3(0.0), 1.0);"
			"      }"
			"   }"
			"   gl_FragColor = vec4(color.rgb * color.a, color.a);"
			"}");
		if (ok) ok = shader->Link();
		if (ok) SetShader(shader);
		if (!ok) delete shader;
		return ok;
	}

	// ------------------------------------------------------------------------
	void SliceRenderer::UpdateViewportPlane()
	{
		double viewportAspect = static_cast<double>(viewportWidth) / 
			static_cast<double>(viewportHeight);
		double sliceAspect = right.length() / up.length();
		if (viewportAspect > sliceAspect)
		{
			// Add margin left and right
			double margin = viewportAspect / sliceAspect - 1.0;
			vporigin = origin - 0.5 * margin * right;
			vpright = right + margin * right;
			vpup = up;
		}
		else
		{
			// Add margin above and below
			double margin = sliceAspect / viewportAspect - 1.0;
			vporigin = origin - 0.5 * margin * up;
			vpright = right;
			vpup = up + margin * up;
		}
	}
}
