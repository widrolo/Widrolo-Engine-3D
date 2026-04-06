#pragma once
#include "Engine/Types/CommonTypes.h"

struct GPUSettingsOpenGL
{

};

struct GPUSettingsVulkan
{
    _GLOBAL_CEX_ bool useWAllocator = true;
    _GLOBAL_CEX_ bool enableValidation = true;
};
