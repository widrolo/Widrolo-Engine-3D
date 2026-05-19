#pragma once
#include "Engine/Types/CommonTypes.h"

struct GPUSettingsOpenGL
{

};

struct GPUSettingsVulkan
{
    enum class InvalidResultAction
    {
        LetGo,
        Stall,
        Abort
    };
    _GLOBAL_CEX_ bool useWAllocator = true;
    _GLOBAL_CEX_ bool enableValidation = true;
    _GLOBAL_CEX_ InvalidResultAction invalidResultAction = InvalidResultAction::Abort;
};
