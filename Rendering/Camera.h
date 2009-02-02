#pragma once

#include "GLBlaat/GL.h"
#include "GLBlaat/GLResource.h"
#include "Renderable.h"
#include "Math/Vector3.h"

#include <algorithm>
#include <limits>

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

		Camera() : position(0.0, 0.0, -5.0), focus(), up(0.0, 1.0, 0.0) { };

		virtual void SetZPlanes(double *bounds)
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
						Vector3 p = Vector3(bounds[x], bounds[y+2], bounds[z+4]) - position;
						double depth = p.dot(viewvec);
						if (depth < minDepth) minDepth = depth;
						if (depth > maxDepth) maxDepth = depth;
					}
				}
			}
			// Set Z planes
			nearZ = std::max(1.0, minDepth);
			farZ = std::min(maxDepth, std::numeric_limits<double>::max());
		}

		virtual void FocusOn(const Renderable *obj)
		{
			focus = obj->GetCenter();
		}

		// For use by more complicated camera controls
		// Update base camera info
		virtual void Update() { };

		virtual void Draw()
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

	private:
		// Not implemented
		Camera(const Camera&);
		void operator=(const Camera&);
	};
}
