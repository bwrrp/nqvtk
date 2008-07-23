#pragma once

#include "Renderer.h"

#include "GLBlaat/GLUtility.h"

#include <QObject> // for qDebug
#include <cassert>


namespace NQVTK
{
	class SimpleRenderer : public NQVTK::Renderer
	{
	public:
		typedef Renderer Superclass;

		SimpleRenderer() { }

		virtual void UpdateLighting()
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

		virtual void Draw()
		{
			Clear();
			DrawCamera();
			UpdateLighting();
			DrawRenderables();
			GLUtility::CheckOpenGLError("SimpleRenderer::Draw()");
		}

	private:
		// Not implemented
		SimpleRenderer(const SimpleRenderer&);
		void operator=(const SimpleRenderer&);
	};
}
