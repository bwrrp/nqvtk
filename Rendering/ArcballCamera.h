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

		ArcballCamera()
		{
			active = false;
		}

		virtual void StartArcball(Vector3 from)
		{
			initialPosition = position;
			initialUp = up;
			dragFrom = from.normalized();
			rotation = Matrix3x3::identity();
			active = true;
		}

		virtual void StopArcball()
		{
			// Update final position and orientation
			Update();
			active = false;
		}

		virtual void SetRotation(Vector3 dragTo)
		{
			Vector3 to = dragTo.normalized();
			Vector3 axis = to.cross(dragFrom);
			double sinangle = axis.length();
			axis = axis / sinangle;
			double cosangle = to.dot(dragFrom);
			// Rotation matrix from http://en.wikipedia.org/wiki/Rotation_matrix
			// (counterclockwise rotation around an arbitrary normalized vector)
			double rotationMatrix[3][3] = {
				{axis.x * axis.x + (1.0 - axis.x * axis.x) * cosangle, 
					axis.x * axis.y * (1.0 - cosangle) - axis.z * sinangle, 
					axis.x * axis.z * (1.0 - cosangle) + axis.y * sinangle}, 
				{axis.x * axis.y * (1.0 - cosangle) + axis.z * sinangle, 
					axis.y * axis.y + (1.0 - axis.y * axis.y) * cosangle, 
					axis.y * axis.z * (1.0 - cosangle) - axis.x * sinangle}, 
				{axis.x * axis.z * (1.0 - cosangle) - axis.y * sinangle, 
					axis.y * axis.z * (1.0 - cosangle) + axis.x * sinangle, 
					axis.z * axis.z + (1.0 - axis.z * axis.z) * cosangle}};
			rotation = Matrix3x3(rotationMatrix);
		}

		virtual void Update()
		{
			if (active)
			{
				position = initialPosition * rotation;
				up = initialUp * rotation;
			}
		}

	protected:
		bool active;
		Vector3 initialPosition;
		Vector3 initialUp;
		Vector3 dragFrom;
		Matrix3x3 rotation;

	private:
		// Not implemented
		ArcballCamera(const ArcballCamera&);
		void operator=(const ArcballCamera&);
	};
}
