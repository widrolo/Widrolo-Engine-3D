#pragma once
#include <string>

#include "Engine/Types/CommonTypes.h"
#include "Engine/Types/Audio/Echo/Resource.h"
#include "Engine/WTL/vector.h"

struct AudioSound
{
    std::string name;
    byte* soundData;
    sizeT sizeBytes;
    uint32 sampleRate;
    Echo::ChannelLayout channelLayout;
    bool isLoaded;
};

struct AudioStream
{
    std::string name;
    uint32 sampleRate;
    Echo::ChannelLayout channelLayout;
    bool isLoaded;
};

struct AudioStreamContent
{
    const AudioStream* stream;
    byte* soundData;
    sizeT playHead;
};

struct VirtualAudio;
struct AudioBus
{
    std::string name;
    float32 volume = 1.0f;
    float32 pitch = 1.0f;
    wtl::vector<VirtualAudio*> children;
};

enum class VirtualAudioType : uint8
{
    Static,
    Stream
};

struct VirtualAudio
{
    union
    {
        Echo::AudioSoundHandle sound;
        Echo::AudioStreamHandle stream;
    } data;
    AudioBus* parent;
    // every virt audio needs its own play head for streamed audio
    AudioStreamContent* content;
    // this is different front the stream content head, this is the play head on the track, not streamed data.
    sizeT playHead;
    float32 volume;
    float32 pitch;
    VirtualAudioType type;
    bool isPlaying : 1;
    bool isLooping : 1;
};

