#pragma once

#include "GLBlaat/GL.h"
#include "Camera.h"

#include <algorithm>

namespace NQVTK 
{
	class OrthoCamera : public Camera
	{
	public:
		typedef Camera Superclass;

		double width;
		double height;
		bool preserveAspect;

		OrthoCamera()
		{
			width = height = 100.0;
			preserveAspect = false;
		}

		virtual void FocusOn(const Renderable *obj)
		{
			// Set focus
			Superclass::FocusOn(obj);

			// Get object info
			double bounds[6];
			obj->GetBounds(bounds);

			double size[3];
			for (int i = 0; i < 3; ++i)
			{
				size[i] = bounds[2 * i + 1] - bounds[2 * i];
			}

			// TODO: calculate actual size of bounding box instead
			radius = 0.9 * std::max(size[0], std::max(size[1], size[2]));

			width = 2.0 * radius;
			height = 2.0 * radius;

			if (preserveAspect) width *= aspect;
		}

		virtual void Draw()
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(-width / 2.0, width / 2.0, -height / 2.0, height / 2.0, nearZ, farZ);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(
				position.x, position.y, position.z, 
				focus.x, focus.y, focus.z, 
				up.x, up.y, up.z);
		}

	protected:
		double radius;

	private:
		// Not implemented
		OrthoCamera(const OrthoCamera&);
		void operator=(const OrthoCamera&);
	};
}
