#pragma once

// bitmask maker
#define BIT(bit) (1<<(bit))

/**
 * Sets a specific bit in a given integer type to 1.
 * @param field Pointer to the integer value whose bit will be set.
 * @param bit The position of the bit to be set.
 */
template<typename T>
inline void SetBit(T* field, const unsigned long bit)
{
	*field = *field | BIT(bit);
}

/**
 * Clears a specific bit in a given integer type to 0.
 * @param field Pointer to the integer value whose bit will be cleared.
 * @param bit The position of the bit to be cleared.
 */
template<typename T> // preferably some kind of integer
inline void ClearBit(T* field, const unsigned long bit)
{
	// Black magic territory
	*field = *field &~ BIT(bit);
}

/**
 * Checks if a specific bit in a given integer type is set to 1.
 * @param field The integer value whose bit will be checked.
 * @param bit The position of the bit to be checked.
 * @return True if the bit is set, false otherwise.
 */
template<typename T> // preferably some kind of integer
inline bool CheckBitSet(const T field, const unsigned long bit)
{
	// Real black magic territory
	return (field & BIT(bit)) != 0;
}
