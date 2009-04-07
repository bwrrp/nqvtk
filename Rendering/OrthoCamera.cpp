#pragma once

#include "OrthoCamera.h"

#include "Renderables/Renderable.h"
#include "Math/Vector3.h"

#include "GLBlaat/GL.h"

#include <algorithm>

namespace NQVTK 
{
	// ------------------------------------------------------------------------
	OrthoCamera::OrthoCamera()
	{
		width = height = 100.0;
		preserveAspect = false;
	}

	// ------------------------------------------------------------------------
	void OrthoCamera::FocusOn(const Renderable *obj)
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

	// ------------------------------------------------------------------------
	void OrthoCamera::Draw()
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-width / 2.0, width / 2.0, 
			-height / 2.0, height / 2.0, 
			nearZ, farZ);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(
			position.x, position.y, position.z, 
			focus.x, focus.y, focus.z, 
			up.x, up.y, up.z);
	}
}
