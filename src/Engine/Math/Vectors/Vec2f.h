#pragma once

#include <Engine/Types/CommonTypes.h>
#include <Engine/Math/Math.h>
#include <format>

namespace WEngine
{
	struct Vector2
	{
		float32 x;
		float32 y;

		constexpr Vector2() : x(0), y(0) {}
		constexpr Vector2(std::array<float32, 2> vals) : x(vals[0]), y(vals[1]) {}
		constexpr Vector2(float32 x, float32 y) : x(x), y(y) {}

		~Vector2() = default;

		// Returns a 0 vector
		const static Vector2 Zero;
		const static Vector2 One;

		// Vector to vector operators
		Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
		Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
		Vector2 operator*(const Vector2& other) const { return Vector2(x * other.x, y * other.y); }
		Vector2 operator/(const Vector2& other) const
		{
			if (other.x == 0 || other.y == 0)
				return Vector2::Zero;
			return Vector2(x / other.x, y / other.y);
		}

		// Vector to float operators
		Vector2 operator+(const float32& other) const { return Vector2(x + other, y + other); }
		Vector2 operator-(const float32& other) const { return Vector2(x - other, y - other); }
		Vector2 operator*(const float32& other) const { return Vector2(x * other, y * other); }
		Vector2 operator/(const float32& other) const
		{
			if (other == 0)
				return Vector2::Zero;
			return Vector2(x / other, y / other);
		}

		bool operator==(const Vector2& other) const { return x == other.x && y == other.y; }
		bool operator!=(const Vector2& other) const { return !(*this == other); }

		bool operator<(const Vector2& other) const
		{
			return SqrMagnitude(*this) < SqrMagnitude(other);
		}
		bool operator>(const Vector2& other) const
		{
			return SqrMagnitude(*this) > SqrMagnitude(other);
		}
		bool operator<=(const Vector2& other) const
		{
			return SqrMagnitude(*this) <= SqrMagnitude(other);
		}
		bool operator>=(const Vector2& other) const
		{
			return SqrMagnitude(*this) >= SqrMagnitude(other);
		}

		// Returns a normalised vector
		static Vector2 Normalised(const Vector2& vec)
		{
			float32 length = Magnitude(vec);
			if (length == 0)
				length = 0.001f; // approach 0
			Vector2 retVec2 = {
				vec.x / length,
				vec.y / length
			};

			return retVec2;
		}

		// Normalises the current vector
		void NormaliseThis()
		{
			float32 length = Magnitude(Vector2(x, y));
			if (length == 0)
				length = 0.001f; // approach 0
			x /= length;
			y /= length;
		}

		// Returns the length of the vector
		static inline float32 Magnitude(const Vector2& vec)
		{
			return Math::Sqrt((vec.x * vec.x) + (vec.y * vec.y));
		}
		// Returns the Magnitude faster
		static inline float32 SqrMagnitude(const Vector2& vec)
		{
			return (vec.x * vec.x) + (vec.y * vec.y);
		}
	};
}

template<>
struct std::formatter<WEngine::Vector2>
{
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx)
	{
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const WEngine::Vector2& v, FormatContext& ctx) const
	{
		return format_to(ctx.out(), "[{} | {}]", v.x, v.y);
	}
};