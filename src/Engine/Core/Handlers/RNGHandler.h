#pragma once
#include <random>
#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
	struct Vector2;

	/**
	 * This Handler provides methods to generate random numbers of different types and within specified ranges.
	 */
	class RNGHandler
	{
	public:
		RNGHandler();
		~RNGHandler() = default;

	private:
		std::random_device m_rd;
		std::mt19937 m_rng;

	public:
		/**
		 * Generates a random integer within the specified range [min, max].
		 * @param min The minimum value of the range (inclusive).
		 * @param max The maximum value of the range (inclusive).
		 * @return A random integer within the specified range.
		 */
		int64 GetRandomInt(int64 min, int64 max);

		/**
		 * Generates a random floating-point number within the specified range [min, max].
		 * @param min The minimum value of the range (inclusive).
		 * @param max The maximum value of the range (inclusive).
		 * @return A random floating-point number within the specified range.
		 */
		float32 GetRandomFloat(float32 min, float32 max);

		/**
		 * Generates a random 2D vector with a length within the specified range [0, maxLength].
		 * @param maxLength The maximum length of the generated vector.
		 * @return A random 2D vector with a length within the specified range.
		 */
		Vector2 GetRandomVector2(float32 maxLength);

	};
}
