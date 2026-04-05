#pragma once

#include <exception>

namespace WEngine
{
	class IgnoredNullableCheckException : public std::exception
	{
	public:
		const char* what() 
		{
			return "Received a Nullable but never checked if the value can be read.";
		}
	};
	/**
	 * A template class for storing an optional value of type T.
	 * This can be used to represent a nullable type, providing a safe
	 * and explicit way to handle missing values.
	 */
	template<typename T>
	class Nullable
	{
	public:
		/**
		 * Default constructor for the Nullable class. Initializes an instance with no value.
		 */
		Nullable() : m_value(), m_hasValue(false) {}
		/**
		 * Constructor for the Nullable class that initializes an instance with a specified value.
		 * @param value The value to be stored in this instance.
		 */
		Nullable(T value) : m_value(value), m_hasValue(true) {}

		/**
		 * Checks if this instance has a valid value.
		 * @return True if this instance contains a value, false otherwise.
		 */
		[[nodiscard]] bool HasValue() const { return m_hasValue; }

		/**
		 * Retrieves the value stored in this instance. If no value is present, the behavior is undefined.
		 * Make sure to check HasValue() before calling this method.
		 * @return The value stored in this instance.
		 */
		T GetValue() const { return m_value; }
	private:
		T m_value;
		bool m_hasValue;
	};
}