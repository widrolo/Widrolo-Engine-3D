#include "Callback.h"

#include <cmath>
#include <cstring>

#include "Engine/Util/Log.h"

void EchoMiniAudioCallback(ma_device *dev, void *output, const void *input, uint32 frame_count)
{
    return;
    // just for testing
    static uint32 phase = 0;

    const float freq = 150.0f;
    const uint32 channels = dev->playback.channels;
    const uint32 bytesPerSample = ma_get_bytes_per_sample(dev->playback.format);

    memset(output, 0, (sizeT)frame_count * channels * bytesPerSample);

    for (uint32 i = 0; i < frame_count; ++i)
    {
        const float32 circle = 2.0f * std::numbers::pi;
        const float32 sample = sinf(circle * freq * (float32)phase / (float32)dev->sampleRate) * 0.2f;
        ++phase;

        for (uint32 channel = 0; channel < channels; ++channel)
        {
            byte* dest = (byte*)output + ((sizeT)i * channels + channel) * bytesPerSample;

            switch (dev->playback.format)
            {
                case ma_format_f32:
                {
                    float32* typed = (float32*)dest;
                    *typed = sample;
                    break;
                }
                case ma_format_s16:
                {
                    int16* typed = (int16*)dest;
                    *typed = (int16)(sample * (float32)max_int16);
                    break;
                }
                case ma_format_s32:
                {
                    int32* typed = (int32*)dest;
                    *typed = (int32)(sample * (float32)max_int32);
                    break;
                }
                case ma_format_u8:
                {
                    uint8* typed = dest;
                    *typed = (uint8)((sample + 1.0f) * 0.5f * (float32)max_uint8);
                    break;
                }
                default:
                    break;
            }
        }
    }
}
