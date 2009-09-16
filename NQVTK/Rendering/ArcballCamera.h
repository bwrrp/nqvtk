#pragma once

#include "Camera.h"

#include "Math/Vector3.h"
#include "Math/Matrix3x3.h"

namespace NQVTK
{
	class ArcballCamera : public Camera
	{
	public:
		typedef Camera Superclass;

		ArcballCamera();

		virtual void StartArcball(Vector3 from);
		virtual void StopArcball();

		virtual void SetRotation(Vector3 dragTo);

		virtual void Update();

		virtual void Draw();

	protected:
		bool active;
		Vector3 initialPosition;
		Vector3 initialUp;
		Vector3 dragFrom;
		Matrix3x3 rotation;
		Matrix3x3 initialFrame;

	private:
		// Not implemented
		ArcballCamera(const ArcballCamera&);
		void operator=(const ArcballCamera&);
	};
}
