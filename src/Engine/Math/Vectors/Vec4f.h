#pragma once
#include <Engine/Types/CommonTypes.h>
#include <Engine/Math/Math.h>

namespace WEngine
{
	struct Vector4
	{
		float32 x;
		float32 y;
		float32 z;
		float32 w;

		constexpr Vector4() : x(0), y(0), z(0), w(0) {}
		constexpr Vector4(const Vector4& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
		constexpr Vector4(float32 x, float32 y, float32 z, float32 w) : x(x), y(y), z(z), w(w) {}

		~Vector4() = default;

		// Returns a 0 vector
		const static Vector4 Zero;
		const static Vector4 One;

		// Vector to vector operators
		Vector4 operator+(const Vector4& other) const { return Vector4(x + other.x, y + other.y, z + other.z, w + other.w); }
		Vector4 operator-(const Vector4& other) const { return Vector4(x - other.x, y - other.y, z - other.z, w - other.w); }
		Vector4 operator*(const Vector4& other) const { return Vector4(x * other.x, y * other.y, z * other.z, w * other.w); }
		Vector4 operator/(const Vector4& other) const
		{
			if (other.x == 0 || other.y == 0 || other.z == 0 || other.w == 0)
				return Vector4::Zero;
			return Vector4(x / other.x, y / other.y, z / other.z, w / other.w);
		}

		// Vector to float operators
		Vector4 operator+(const float32& other) const { return Vector4(x + other, y + other, z + other, w + other); }
		Vector4 operator-(const float32& other) const { return Vector4(x - other, y - other, z - other, w - other); }
		Vector4 operator*(const float32& other) const { return Vector4(x * other, y * other, z * other, w * other); }
		Vector4 operator/(const float32& other) const
		{
			if (other == 0)
				return Vector4::Zero;
			return Vector4(x / other, y / other, z / other, w / other);
		}

		bool operator==(const Vector4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
		bool operator!=(const Vector4& other) const { return !(*this == other); }

		bool operator<(const Vector4& other) const
		{
			return SqrMagnitude(*this) < SqrMagnitude(other);
		}
		bool operator>(const Vector4& other) const
		{
			return SqrMagnitude(*this) > SqrMagnitude(other);
		}
		bool operator<=(const Vector4& other) const
		{
			return SqrMagnitude(*this) <= SqrMagnitude(other);
		}
		bool operator>=(const Vector4& other) const
		{
			return SqrMagnitude(*this) >= SqrMagnitude(other);
		}

		// Returns a normalised vector
		static Vector4 Normalised(const Vector4& vec)
		{
			float32 length = Magnitude(vec);
			if (length == 0)
				length = 0.001f; // approach 0
			Vector4 retVec2 = {
				vec.x / length,
				vec.y / length,
				vec.z / length,
				vec.w / length
			};

			return retVec2;
		}

		// Normalises the current vector
		void NormaliseThis()
		{
			float32 length = Magnitude(Vector4(x, y, z, w));
			if (length == 0)
				length = 0.001f; // approach 0
			x /= length;
			y /= length;
			z /= length;
		}

		// Returns the length of the vector
		static inline float32 Magnitude(const Vector4& vec)
		{
			return Math::Sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z) + (vec.w * vec.w));
		}
		// Returns the Magnitude faster
		static inline float32 SqrMagnitude(const Vector4& vec)
		{
			return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z) + (vec.w * vec.w);
		}
	};
}
