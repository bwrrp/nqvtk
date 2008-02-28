#pragma once

#include "Renderable.h"

#include "GLBlaat/GLBuffer.h"

namespace NQVTK
{
	class VBOMesh : public Renderable
	{
	public:
		typedef Renderable Superclass;

		VBOMesh()
		{
		}

		virtual ~VBOMesh() { }

		virtual void Draw()
		{
			// TODO: render VBOs
			glBegin(GL_TRIANGLES);
			glColor3d(1.0, 0.0, 0.0);
			glVertex3d(-1.0, -1.0, 0.0);
			glColor3d(0.0, 1.0, 0.0);
			glVertex3d(1.0, -1.0, 0.0);
			glColor3d(0.0, 0.0, 1.0);
			glVertex3d(0.0, 1.0, 0.0);
			glEnd();
		}

	protected:
		GLBuffer *vbo;
	};
}
