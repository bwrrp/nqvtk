#pragma once

#include <cmath>

namespace NQVTK 
{
	// Meh, prototype hell...
	class Vector3d;
	const Vector3d operator/(const Vector3d &v, double f);

	class Vector3d
	{
	public:
		// Vector elements
		union 
		{
			struct
			{
				double x;
				double y;
				double z;
			};
			double V[3];
		};

		Vector3d() : x(0), y(0), z(0) { };
		Vector3d(double x, double y, double z) : x(x), y(y), z(z) { };

		double length() const
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		const Vector3d normalized() const
		{
			double l = length();
			return *this / l;
		}

		double dot(const Vector3d &v2) const
		{
			return x * v2.x + y * v2.y + z * v2.z;
		}

		const Vector3d cross(const Vector3d &v2) const
		{
			return Vector3d(
				y * v2.z - z * v2.y,
				z * v2.x - x * v2.z,
				x * v2.y - y * v2.x);
		}

		Vector3d &operator+=(const Vector3d &v2)
		{
			x += v2.x;
			y += v2.y;
			z += v2.z;
			return *this;
		}

		Vector3d &operator-=(const Vector3d &v2)
		{
			x -= v2.x;
			y -= v2.y;
			z -= v2.z;
			return *this;
		}

		Vector3d &operator*=(double f)
		{
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}
	};

	inline const Vector3d operator+(const Vector3d &v1, const Vector3d &v2)
	{
		return Vector3d(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
	}

	inline const Vector3d operator-(const Vector3d &v1, const Vector3d &v2)
	{
		return Vector3d(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	}

	inline const Vector3d operator*(double f, const Vector3d &v)
	{
		return Vector3d(f * v.x, f * v.y, f * v.z);
	}

	inline const Vector3d operator*(const Vector3d &v, double f)
	{
		return f * v;
	}

	inline const Vector3d operator/(double f, const Vector3d &v)
	{
		return Vector3d(f / v.x, f / v.y, f / v.z);
	}

	inline const Vector3d operator/(const Vector3d &v, double f)
	{
		// Gracefully handle division by zero
		if (f == 0.0f) return Vector3d();
		double g = 1.0 / f;
		return g * v;
	}
}
