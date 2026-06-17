#include "TextureSwizzler.h"

#include <cstring>

#include "Log.h"
#include "Engine/Core/System/Memory.h"

using namespace WEngine;

void TextureSwizzler::AddSource(const TextureInfo *texture, uint8 sourceChannel, uint8 targetChannel)
{
    if (sourceChannel > 3 || targetChannel > 3)
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Swizzle init error, channel out of range!");
        return;
    }

    if (texture == nullptr)
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Swizzle init error, null texture passed!");
        return;
    }

    if (texture->data == nullptr)
    {
        WLog::SetConsoleError();
        WLog::ConsoleLog("Swizzle init error, texture with null data passed!");
        return;
    }

    m_source[targetChannel] = {texture, sourceChannel};
}

bool TextureSwizzler::Swizzle()
{


    if (!ChannelCheck())
        return false;

    if (!SizeCheck())
        return false;

    if (!CreateTexture())
        return false;

    for (int i = 0; i < 4; i++)
        SwizzleChannel(i);

    return true;
}

TextureInfo TextureSwizzler::RetrieveResult()
{
    return m_outTexture;
}

bool TextureSwizzler::ChannelCheck()
{
    for (const auto& tex : m_source)
    {
        if (tex.texture == nullptr)
        {
            WLog::SetConsoleError();
            WLog::ConsoleLog("Swizzle error, missing channel!");
            return false;
        }
    }
    return true;
}

bool TextureSwizzler::SizeCheck()
{
    for (const auto& tex : m_source)
    {
        if (m_outTexture.height == 0 || m_outTexture.width == 0)
        {
            m_outTexture.height = tex.texture->height;
            m_outTexture.width = tex.texture->width;
        }
        if (m_outTexture.height != tex.texture->height || m_outTexture.width != tex.texture->width)
        {
            WLog::SetConsoleError();
            WLog::ConsoleLog("Swizzle error, not all input is of same size!");
            return false;
        }
    }
    return true;
}

bool TextureSwizzler::CreateTexture()
{
    uint64 size = m_outTexture.height * m_outTexture.width * 4;
    uint8* textureData = wNewArr(uint8, size);

    if (textureData == nullptr)
        return false;

    m_outTexture.data = textureData;

    return true;
}

void TextureSwizzler::SwizzleChannel(uint8 channel)
{
    // legit no need for error checking at this point.

    // So it turns out that images are stored as RGBA RGBA RGBA, not RRR...GGG...BBB...AAA...
    // No memcpy today :(

    uint64 pixelCount = m_outTexture.height * m_outTexture.width;
    auto source = m_source[channel];
    for (uint64 i = 0; i < pixelCount; i++)
        m_outTexture.data[i * 4 + channel] = source.texture->data[i * 4 + source.channel];
}
