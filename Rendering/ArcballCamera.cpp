#pragma once

#include "ArcballCamera.h"

#include "Math/Vector3.h"
#include "Math/Matrix3x3.h"

namespace NQVTK
{
	ArcballCamera::ArcballCamera()
	{
		active = false;
	}

	void ArcballCamera::StartArcball(Vector3 from)
	{
		// Store initial camera frame
		initialPosition = position;
		initialUp = up;
		Vector3 z = (focus - position).normalized();
		Vector3 y = -up.normalized();
		Vector3 x = z.cross(y).normalized();
		y = z.cross(x);
		initialFrame = Matrix3x3::fromCols(x, y, z);
		// Store drag parameters
		dragFrom = initialFrame * from.normalized();
		rotation = Matrix3x3::identity();
		// We're dragging the arcball
		active = true;
	}

	void ArcballCamera::StopArcball()
	{
		// Update final position and orientation
		Update();
		active = false;
	}

	void ArcballCamera::SetRotation(Vector3 dragTo)
	{
		Vector3 to = initialFrame * dragTo.normalized();
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

	void ArcballCamera::Update()
	{
		if (active)
		{
			Vector3 relpos = initialPosition - focus;
			position = focus + rotation * relpos;
			up = rotation * initialUp;
		}
	}

	void ArcballCamera::Draw()
	{
		Update();
		Superclass::Draw();
	}
}
