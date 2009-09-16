#pragma once

#include "Camera.h"

namespace NQVTK 
{
	class OrthoCamera : public Camera
	{
	public:
		typedef Camera Superclass;

		double width;
		double height;
		bool preserveAspect;

		OrthoCamera();

		virtual void FocusOn(const Renderable *obj);

		virtual void Draw();

	protected:
		double radius;

	private:
		// Not implemented
		OrthoCamera(const OrthoCamera&);
		void operator=(const OrthoCamera&);
	};
}
