#pragma once

#include <Engine/EngineDefines.h>

/**
 * Converts a given number of pixels to metres.
 * @param pixels The number of pixels to be converted.
 * @return The equivalent length in metres.
 */
inline float PixelToMetre(int pixels)
{
	return (float)pixels / EngineSettings::pixelsPerMetre;
}
/**
 * Converts a given length in metres to the equivalent number of pixels.
 * @param metre The length in metres to be converted.
 * @return The equivalent number of pixels.
 */
inline int MetreToPixel(float metre)
{
	return (int)(metre * EngineSettings::pixelsPerMetre);
}