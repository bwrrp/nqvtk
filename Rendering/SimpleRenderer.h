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
			Superclass::UpdateLighting();
			glEnable(GL_COLOR_MATERIAL);
		}

		virtual void Draw()
		{
			Clear();
			DrawCamera();
			UpdateLighting();
			DrawRenderables();
			GLUtility::CheckOpenGLError("SimpleRenderer::Draw()");
		}
	};
}
