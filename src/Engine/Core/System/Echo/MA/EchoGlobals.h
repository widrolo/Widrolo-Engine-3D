#pragma once

#include <miniaudio.h>
#include "Helpers/Types.h"
#include <unordered_map>

inline ma_device device;

inline wtl::vector<AudioSound> loadedSounds;
inline wtl::vector<AudioStream> loadedStreams;
inline wtl::vector<AudioBus> loadedBusses;
inline AudioBus masterBus;

inline std::unordered_map<std::string, sizeT> loadedSoundNames;

inline wtl::vector<VirtualAudio> virtualAudio;
