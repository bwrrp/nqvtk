#pragma once

#include "Camera.h"

namespace NQVTK 
{
	class OrbitCamera : public Camera
	{
	public:
		typedef Camera Superclass;

		double rotateX;
		double rotateY;
		double zoom;

		OrbitCamera();

		virtual void FocusOn(const Renderable *obj);

		virtual void Update();

		virtual void Draw();

	protected:
		double radius;

	private:
		// Not implemented
		OrbitCamera(const OrbitCamera&);
		void operator=(const OrbitCamera&);
	};
}
