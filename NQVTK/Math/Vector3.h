#pragma once

#include <cmath>

namespace NQVTK 
{
	// Meh, prototype hell...
	class Vector3;
	const Vector3 operator/(const Vector3 &v, double f);

	class Vector3
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

		Vector3() : x(0), y(0), z(0) { }
		Vector3(double v) : x(v), y(v), z(v) { }
		Vector3(double x, double y, double z) : x(x), y(y), z(z) { }

		double length() const
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		const Vector3 normalized() const
		{
			double l = length();
			return *this / l;
		}

		double dot(const Vector3 &v2) const
		{
			return x * v2.x + y * v2.y + z * v2.z;
		}

		const Vector3 cross(const Vector3 &v2) const
		{
			return Vector3(
				y * v2.z - z * v2.y,
				z * v2.x - x * v2.z,
				x * v2.y - y * v2.x);
		}

		const Vector3 xzy() const
		{
			return Vector3(x, z, y);
		}

		const Vector3 yxz() const
		{
			return Vector3(y, x, z);
		}

		const Vector3 yzx() const
		{
			return Vector3(y, z, x);
		}

		const Vector3 zxy() const
		{
			return Vector3(z, x, y);
		}

		const Vector3 zyx() const
		{
			return Vector3(z, y, x);
		}

		Vector3 &operator+=(const Vector3 &v2)
		{
			x += v2.x;
			y += v2.y;
			z += v2.z;
			return *this;
		}

		Vector3 &operator-=(const Vector3 &v2)
		{
			x -= v2.x;
			y -= v2.y;
			z -= v2.z;
			return *this;
		}

		Vector3 &operator*=(double f)
		{
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}
	};

	inline const Vector3 operator-(const Vector3 &v)
	{
		return Vector3(-v.x, -v.y, -v.z);
	}

	inline const Vector3 operator+(const Vector3 &v1, const Vector3 &v2)
	{
		return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
	}

	inline const Vector3 operator-(const Vector3 &v1, const Vector3 &v2)
	{
		return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	}

	inline const Vector3 operator*(double f, const Vector3 &v)
	{
		return Vector3(f * v.x, f * v.y, f * v.z);
	}

	inline const Vector3 operator*(const Vector3 &v, double f)
	{
		return f * v;
	}

	inline const Vector3 operator/(double f, const Vector3 &v)
	{
		return Vector3(f / v.x, f / v.y, f / v.z);
	}

	inline const Vector3 operator/(const Vector3 &v, double f)
	{
		// Gracefully handle division by zero
		if (f == 0.0f) return Vector3();
		double g = 1.0 / f;
		return g * v;
	}
}
