#pragma once

#include "SimpleRenderer.h"

#include "Renderables/VBOMesh.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLUtility.h"
#include "GLBlaat/GLProgram.h"

#include <cassert>
#include <vector>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	SimpleRenderer::SimpleRenderer()
		: shader(0)
	{
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
		if (shader)
		{
			if (shaderAttribs.size() > 0)
			{
				NQVTK::VBOMesh *mesh = dynamic_cast<NQVTK::VBOMesh*>(renderable);
				if (mesh) mesh->SetupAttributes(shaderAttribs);
			}
			renderable->ApplyParamSets(shader);
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

		// TODO: update shader params, attrib bindings etc.
		if (shader) shader->Start();

		Clear();
		DrawCamera();
		UpdateLighting();
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
}
