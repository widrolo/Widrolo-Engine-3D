#pragma once

#include <Engine/Types/CommonTypes.h>
#include <Engine/Math/Transform.h>
#include <Engine/Types/Rendering/Color.h>
#include <Engine/WTL/vector.h>
#include <string>
#include <Engine/Types/Nullable.h>
#include <yaml-cpp/yaml.h>


namespace WEngine
{
	/**
	 * Structure to hold arguments for a component.
	 */
	struct ComponentArgs
	{
		uint16 componentTypeId; ///< The ID of the component type.

		YAML::Node componentRoot; ///< The root node of the component in the YAML configuration.

		/**
		 * Fetches an integer value from the component arguments using a specified variable name.
		 * @param varName Name of the variable to fetch.
		 * @return Nullable wrapper around the integer value fetched, or null if not found or invalid type.
		 */
		Nullable<int64> GetIntFromParams(std::string varName);
		/**
	 	 * Fetches a float value from the component arguments using a specified variable name.
	 	 * @param varName Name of the variable to fetch.
	 	 * @return Nullable wrapper around the float value fetched, or null if not found or invalid type.
	 	 */
		Nullable<float32> GetFloatFromParams(std::string varName);
		/**
	  	 * Fetches a boolean value from the component arguments using a specified variable name.
	  	 * @param varName Name of the variable to fetch.
	  	 * @return Nullable wrapper around the boolean value fetched, or null if not found or invalid type.
	  	 */
		Nullable<bool> GetBoolFromParams(std::string varName);
		/**
	 	 * Fetches a string value from the component arguments using a specified variable name.
	 	 * @param varName Name of the variable to fetch.
	 	 * @return Nullable wrapper around the string value fetched, or null if not found or invalid type.
	 	 */
		Nullable<std::string> GetStringFromParams(std::string varName);
		/**
	 	 * Fetches a Vector2 value from the component arguments using a specified variable name.
	 	 * @param varName Name of the variable to fetch.
	 	 * @return Nullable wrapper around the Vector2 value fetched, or null if not found or invalid type.
	 	 */
		Nullable<Vector2> GetVector2FromParams(std::string varName);
		/**
	 	 * Fetches a Color value from the component arguments using a specified variable name.
	 	 * @param varName Name of the variable to fetch.
	 	 * @return Nullable wrapper around the Color value fetched, or null if not found or invalid type.
	 	 */
		Nullable<Color> GetColorFromParams(std::string varName);
	};

	/**
	 * Structure to hold arguments for spawning a game object.
	 */
	struct SpawnArgs
	{
		std::string name; ///< The name of the entity.
		Transform transform; ///< The transformation data of the entity.
		wtl::vector<ComponentArgs> ca; ///< Vector containing component arguments for the entity.
	};
}
