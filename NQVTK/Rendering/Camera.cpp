#include "Camera.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLResource.h"

#include "Renderables/Renderable.h"
#include "Math/Vector3.h"

#include <algorithm>
#include <limits>

namespace NQVTK 
{
	// ------------------------------------------------------------------------
	Camera::Camera() : position(0.0, 0.0, -5.0), focus(), up(0.0, 1.0, 0.0) 
	{
	}

	// ------------------------------------------------------------------------
	void Camera::SetZPlanes(const double *bounds)
	{
		// Project each of the bounding box corners on the view vector
		// and check max / min distance
		Vector3 viewvec = (focus - position).normalized();
		double minDepth = std::numeric_limits<double>::infinity();
		double maxDepth = -minDepth;
		for (int x = 0; x < 2; ++x)
		{
			for (int y = 0; y < 2; ++y)
			{
				for (int z = 0; z < 2; ++z)
				{
					Vector3 p = Vector3(bounds[x], bounds[y+2], 
						bounds[z+4]) - position;
					double depth = p.dot(viewvec);
					if (depth < minDepth) minDepth = depth;
					if (depth > maxDepth) maxDepth = depth;
				}
			}
		}
		// If the boundaries are invalid, use sane default values
		if (minDepth > maxDepth)
		{
			minDepth = 1.0;
			maxDepth = 10.0;
		}
		// Slightly increase the boundaries to prevent accuracy issues
		minDepth *= 0.9;
		maxDepth *= 1.1;
		// Set Z planes
		nearZ = std::max(1.0, minDepth);
		farZ = std::min(maxDepth, std::numeric_limits<double>::max());
	}

	// ------------------------------------------------------------------------
	void Camera::FocusOn(const Renderable *obj)
	{
		// TODO: add a way to set the initial position
		// TODO: zoom in/out to show the full object
		focus = obj->GetCenter();
	}

	// ------------------------------------------------------------------------
	void Camera::Draw()
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
	}
}
