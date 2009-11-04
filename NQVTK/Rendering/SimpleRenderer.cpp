#include "SimpleRenderer.h"

#include "Renderables/VBOMesh.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLUtility.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTextureManager.h"

#include <cassert>
#include <vector>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	SimpleRenderer::SimpleRenderer()
		: shader(0)
	{
		drawBackground = true;
	}

	// ------------------------------------------------------------------------
	SimpleRenderer::~SimpleRenderer()
	{
		delete shader;
	}

	// ------------------------------------------------------------------------
	void SimpleRenderer::UpdateLighting()
	{
		// Update light position
		Superclass::UpdateLighting();
		// Set other lighting parameters for fixed function
		float global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
		float diffuse[]= { 0.5f, 0.5f, 0.5f, 1.0f };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		float specular[] = {1.0f, 1.0f, 1.0f , 1.0f};
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
		float specmat[] = {0.5f, 0.5f, 0.5f, 1.0f};
		glMaterialfv(GL_FRONT, GL_SPECULAR, specmat);
		glMateriali(GL_FRONT, GL_SHININESS, 64);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}

	// ------------------------------------------------------------------------
	void SimpleRenderer::PrepareForRenderable(int objectId, 
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
	void SimpleRenderer::Clear()
	{
		Superclass::Clear();
		if (drawBackground)
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glDisable(GL_LIGHTING);
			glDisable(GL_BLEND);
			glDepthMask(GL_FALSE);
			glBegin(GL_QUADS);
			glColor4d(0.2, 0.2, 0.25, 0.0);
			glVertex3d(-1.0, -1.0, 0.0);
			glVertex3d(1.0, -1.0, 0.0);
			glColor4d(0.6, 0.6, 0.65, 0.0);
			glVertex3d(1.0, 1.0, 0.0);
			glVertex3d(-1.0, 1.0, 0.0);
			glEnd();
			glPopAttrib();
		}
	}

	// ------------------------------------------------------------------------
	void SimpleRenderer::Draw()
	{
		// Prepare for rendering
		if (fboTarget)
		{
			fboTarget->Bind();
		}
		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

		Clear();
		DrawCamera();
		UpdateLighting();

		// TODO: update shader params, attrib bindings etc.
		if (shader) shader->Start();

		DrawRenderables();
		GLUtility::CheckOpenGLError("SimpleRenderer::Draw()");

		if (shader) shader->Stop();

		if (fboTarget)
		{
			fboTarget->Unbind();
		}
	}

	// ------------------------------------------------------------------------
	GLProgram *SimpleRenderer::SetShader(GLProgram *shader)
	{
		GLProgram *oldshader = this->shader;
		this->shader = shader;
		shaderAttribs.clear();
		if (shader) shaderAttribs = shader->GetActiveAttributes();
		return oldshader;
	}

	// ------------------------------------------------------------------------
	void SimpleRenderer::SetDrawBackground(bool draw)
	{
		drawBackground = draw;
	}
}
