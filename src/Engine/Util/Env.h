#pragma once

#include <cstdlib>

#include <string>

/**
 * Retrieves the value of an environment variable.
 * @param variableName The name of the environment variable.
 * @return The value of the environment variable, or an empty string if the variable does not exist.
 * @note This function should not be used in user-facing code, as it may lead to security vulnerabilities.
 */
inline std::string GetEnv(std::string variableName)
{
	std::string val = std::getenv(variableName.c_str());
	return val;
}