#pragma once

#include "Math/Vector3.h"

namespace NQVTK 
{
	class Renderable;

	class Camera
	{
	public:
		Vector3 position;
		Vector3 focus;
		Vector3 up;
		double aspect;
		double nearZ;
		double farZ;

		Camera();

		virtual void SetZPlanes(const double *bounds);

		virtual void FocusOn(const Renderable *obj);

		// For use by more complicated camera controls
		// Update base camera info
		virtual void Update() { };

		virtual void Draw();

	private:
		// Not implemented
		Camera(const Camera&);
		void operator=(const Camera&);
	};
}
