#pragma once
#include <source_location>
#include <iostream>
#include <format>
#include <string_view>

namespace WEngine
{
	/**
	 * A utility class for logging and printing information to the console.
	 * Contains methods for console logging, setting console text colors, and printing source location information.
	 */
	class WLog
	{
	public:
		/**
 		 * Logs a message to the console, with optional source location information.
 		 * @param msg The message to be logged.
 		 * @param location The source location of the log message (defaults to current).
 		 * @note For formatted strings use std::format.
 		 */
		static void ConsoleLog(
			const std::string& msg,
			const std::source_location& location = std::source_location::current())
		{
			PrintInfo(location);
			std::cout << msg << '\n';
			SetConsoleMessage();
		}

		/**
		 * Sets the console text color to a default message color.
		 */
		static void SetConsoleMessage();
		/**
		 * Sets the console text color to an information color.
		 */
		static void SetConsoleInfo();
		/**
		 * Sets the console text color to a warning color.
		 */
		static void SetConsoleWarning();
		/**
		 * Sets the console text color to an error color.
		 */
		static void SetConsoleError();
		/**
		 * Sets the console text color to a success color.
		 */
		static void SetConsoleSuccess();
	private:
		static void PrintInfo(const std::source_location& location);
	};

}
