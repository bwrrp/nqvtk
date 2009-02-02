#pragma once

#include "GLBlaat/GL.h"
#include "Camera.h"

#include <algorithm>

namespace NQVTK 
{
	class OrbitCamera : public Camera
	{
	public:
		typedef Camera Superclass;

		double rotateX;
		double rotateY;
		double zoom;

		OrbitCamera()
		{
			rotateX = rotateY = 0.0;
			zoom = 1.0;

			radius = 1.0;
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

			Update();
		}

		virtual void Update()
		{
			const double DEGREES_TO_RADIANS = 0.0174532925199433;

			double rx = rotateX * DEGREES_TO_RADIANS;
			double ry = rotateY * DEGREES_TO_RADIANS;

			// Update the position on the unit sphere relative to focus
			Vector3 focusToPos(
				sin(ry) * cos(rx), 
				-sin(rx), 
				-cos(ry) * cos(rx));

			// Scale the position to the correct zoom level
			focusToPos *= 3.0 * radius * zoom;

			// Update the position
			position = focus + focusToPos;
		}

		virtual void Draw()
		{
			Update();
			Superclass::Draw();
		}

	protected:
		double radius;

	private:
		// Not implemented
		OrbitCamera(const OrbitCamera&);
		void operator=(const OrbitCamera&);
	};
}
