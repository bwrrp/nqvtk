#pragma once

#include "GLBlaat/GL.h"
#include "GLBlaat/GLResource.h"
#include "Renderable.h"
#include "Math/Vector3.h"

#include <algorithm>

namespace NQVTK 
{
	class Camera : public GLResource
	{
	public:
		typedef GLResource Superclass;

		Vector3 position;
		Vector3 focus;
		Vector3 up;
		double aspect;
		double nearZ;
		double farZ;

		double rotateX;
		double rotateY;
		double zoom;

		Camera() : position(0.0, 0.0, -5.0), focus(), up(0.0, 1.0, 0.0), 
			rotateX(0.0), rotateY(0.0), zoom(0.0) { };

		void FocusOn(const Renderable *obj)
		{
			// Get object info
			double bounds[6];
			obj->GetBounds(bounds);

			double size[3];
			for (int i = 0; i < 3; ++i)
			{
				size[i] = bounds[2 * i + 1] - bounds[2 * i];
			}
			// TODO: calculate actual size of bounding box instead
			double radius = 0.5 * std::max(size[0], std::max(size[1], size[2]));

			// TODO: set focus, nearZ, farZ based on object position & transformation!
			position = Vector3(0.0, 0.0, -3.0 * radius);
			// Our object is moved to 0,0,0
			focus = Vector3();
			nearZ = 2 * radius;
			farZ = 4 * radius;
		}

		void Draw()
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45.0, aspect, nearZ, farZ);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(
				position.x, position.y, position.z, 
				focus.x, focus.y, focus.z, 
				up.x, up.y, up.z);

			// TODO: instead of "world transformations" we should transform position and up
			glTranslated(0.0, 0.0, -10.0 * zoom);
			glRotated(rotateX, 1.0, 0.0, 0.0);
			glRotated(rotateY, 0.0, 1.0, 0.0);
		}

	private:
		// Not implemented
		Camera(const Camera&);
		void operator=(const Camera&);
	};
}
