#pragma once

#include <array>
#include "Engine/Types/Rendering/TextureInfo.h"


namespace WEngine
{
    class TextureSwizzler
    {
    public:
        struct SourceInfo
        {
            const TextureInfo* texture{};
            // 0123 = rgba
            uint8 channel{};
        };
    public:
        void AddSource(const TextureInfo* texture, uint8 sourceChannel, uint8 targetChannel);
        bool Swizzle();

        TextureInfo RetrieveResult();

    private:
        bool ChannelCheck();
        bool SizeCheck();
        bool CreateTexture();

        void SwizzleChannel(uint8 channel);

    private:
        std::array<SourceInfo, 4> m_source;
        TextureInfo m_outTexture{};
    };
}

