#pragma once

#include <Engine/Types/CommonTypes.h>
#include <Engine/Math/Math.h>
#include <array>

namespace WEngine
{
	struct Vector1
	{
		float32 x;

		constexpr Vector1() : x(0) {}
		constexpr Vector1(std::array<float32, 1> vals) : x(vals[0]) {}
		constexpr Vector1(float32 x) : x(x) {}

		~Vector1() = default;

		// Returns a 0 vector
		const static Vector1 Zero;
		const static Vector1 One;

		// Vector to vector operators
		Vector1 operator+(const Vector1& other) const { return Vector1(x + other.x); }
		Vector1 operator-(const Vector1& other) const { return Vector1(x - other.x); }
		Vector1 operator*(const Vector1& other) const { return Vector1(x * other.x); }
		Vector1 operator/(const Vector1& other) const
		{
			if (other.x == 0)
				return Vector1::Zero;
			return Vector1(x / other.x);
		}

		// Vector to float operators
		Vector1 operator+(const float32& other) const { return Vector1(x + other); }
		Vector1 operator-(const float32& other) const { return Vector1(x - other); }
		Vector1 operator*(const float32& other) const { return Vector1(x * other); }
		Vector1 operator/(const float32& other) const
		{
			if (other == 0)
				return Vector1::Zero;
			return Vector1(x / other);
		}

		bool operator==(const Vector1& other) const { return x == other.x; }
		bool operator!=(const Vector1& other) const { return !(*this == other); }

		bool operator<(const Vector1& other) const
		{
			return SqrMagnitude(*this) < SqrMagnitude(other);
		}
		bool operator>(const Vector1& other) const
		{
			return SqrMagnitude(*this) > SqrMagnitude(other);
		}
		bool operator<=(const Vector1& other) const
		{
			return SqrMagnitude(*this) <= SqrMagnitude(other);
		}
		bool operator>=(const Vector1& other) const
		{
			return SqrMagnitude(*this) >= SqrMagnitude(other);
		}

		// Returns a normalised vector
		static Vector1 Normalised(const Vector1& vec)
		{
			float32 length = Magnitude(vec);
			if (length == 0)
				length = 0.001f; // approach 0
			Vector1 retVec2 = {
				vec.x / length
			};

			return retVec2;
		}

		// Normalises the current vector
		void NormaliseThis()
		{
			float32 length = Magnitude(Vector1(x));
			if (length == 0)
				length = 0.001f; // approach 0
			x /= length;
		}

		// Returns the length of the vector
		static inline float32 Magnitude(const Vector1& vec)
		{
			return Math::Sqrt((vec.x * vec.x));
		}
		// Returns the Magnitude faster
		static inline float32 SqrMagnitude(const Vector1& vec)
		{
			return (vec.x * vec.x);
		}
	};

}