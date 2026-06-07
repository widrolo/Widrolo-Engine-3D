#pragma once
#include "Engine/Types/CommonTypes.h"

struct GPUSettings
{
    _GLOBAL_CEX_ uint64 stationaryInstBufferSize = 64 * MB;
};

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
#ifdef DEBUG
    _GLOBAL_CEX_ bool enableValidation = true;
#else
    _GLOBAL_CEX_ bool enableValidation = false;
#endif
    _GLOBAL_CEX_ InvalidResultAction invalidResultAction = InvalidResultAction::Abort;
    _GLOBAL_CEX_ uint64 maxInstanceBufferSize = 4;
    _GLOBAL_CEX_ uint64 maxMaterialCount = 512;
};
