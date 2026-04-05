#pragma once
#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
	/**
	 * 32-bit color.
	 */
	struct Color
	{
		Color():
			red(0), green(0), blue(0), alpha(255) {}
		/**
		 * @param r red color.
		 * @param g green color.
		 * @param b blue color.
		 * @param a transparency.
		 */
		Color(uint8 r, uint8 g, uint8 b, uint8 a = 255) :
			red(r), green(g), blue(b), alpha(a) {}

		/**
		 * Takes color as 32 bits; allows for hex inputs.
		 * @param rgba 32-bit color code.
		 */
		Color(uint32 rgba);


		/**
		 * Red channel.
		 */
		uint8 red;
		/**
		 * Green channel.
		 */
		uint8 green;
		/**
		 * Blue channel.
		 */
		uint8 blue;
		/**
		 * Transparency channel.
		 */
		uint8 alpha;

		/***********************************************
		* Pre defined colors.
		***********************************************/

		const static Color White;
		const static Color Black;
		const static Color Red;
		const static Color Green;
		const static Color Blue;
	};
}
