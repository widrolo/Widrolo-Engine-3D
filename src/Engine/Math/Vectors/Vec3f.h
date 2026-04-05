#pragma once

#include <Engine/Types/CommonTypes.h>
#include <Engine/Math/Math.h>

namespace WEngine
{
	struct Vector3
	{
		float32 x;
		float32 y;
		float32 z;

		constexpr Vector3() : x(0), y(0), z(0) {}
		constexpr Vector3(const Vector3& other) : x(other.x), y(other.y), z(other.z) {}
		constexpr Vector3(float32 x, float32 y, float32 z) : x(x), y(y), z(z) {}

		~Vector3() = default;

		// Returns a 0 vector
		const static Vector3 Zero;
		const static Vector3 One;

		// Vector to vector operators
		Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
		Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
		Vector3 operator*(const Vector3& other) const { return Vector3(x * other.x, y * other.y, z * other.z); }
		Vector3 operator/(const Vector3& other) const
		{
			if (other.x == 0 || other.y == 0 || other.z == 0)
				return Vector3::Zero;
			return Vector3(x / other.x, y / other.y, z / other.z);
		}

		// Vector to float operators
		Vector3 operator+(const float32& other) const { return Vector3(x + other, y + other, z + other); }
		Vector3 operator-(const float32& other) const { return Vector3(x - other, y - other, z - other); }
		Vector3 operator*(const float32& other) const { return Vector3(x * other, y * other, z * other); }
		Vector3 operator/(const float32& other) const
		{
			if (other == 0)
				return Vector3::Zero;
			return Vector3(x / other, y / other, z / other);
		}

		bool operator==(const Vector3& other) const { return x == other.x && y == other.y && z == other.z; }
		bool operator!=(const Vector3& other) const { return !(*this == other); }

		bool operator<(const Vector3& other) const
		{
			return SqrMagnitude(*this) < SqrMagnitude(other);
		}
		bool operator>(const Vector3& other) const
		{
			return SqrMagnitude(*this) > SqrMagnitude(other);
		}
		bool operator<=(const Vector3& other) const
		{
			return SqrMagnitude(*this) <= SqrMagnitude(other);
		}
		bool operator>=(const Vector3& other) const
		{
			return SqrMagnitude(*this) >= SqrMagnitude(other);
		}

		// Returns a normalised vector
		static Vector3 Normalised(const Vector3& vec)
		{
			float32 length = Magnitude(vec);
			if (length == 0)
				length = 0.001f; // approach 0
			Vector3 retVec2 = {
				vec.x / length,
				vec.y / length,
				vec.z / length
			};

			return retVec2;
		}

		// Normalises the current vector
		void NormaliseThis()
		{
			float32 length = Magnitude(Vector3(x, y, z));
			if (length == 0)
				length = 0.001f; // approach 0
			x /= length;
			y /= length;
			z /= length;
		}

		// Returns the length of the vector
		static inline float32 Magnitude(const Vector3& vec)
		{
			return Math::Sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
		}
		// Returns the Magnitude faster
		static inline float32 SqrMagnitude(const Vector3& vec)
		{
			return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
		}
	};

}
