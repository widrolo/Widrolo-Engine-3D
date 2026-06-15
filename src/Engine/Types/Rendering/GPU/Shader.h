#pragma once

#include <Engine/Types/CommonTypes.h>

namespace WEngine
{
	/**
 	 * Handle for a shader programm.
 	 */
	struct Shader
	{
		Shader() : handle(0) {};
		Shader(uint64 handle) : handle(handle) {};
		operator uint64() const { return handle; }
		uint64 handle;
	};
}


