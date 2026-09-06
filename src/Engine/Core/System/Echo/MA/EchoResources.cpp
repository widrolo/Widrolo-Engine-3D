#if ECHO_BACKEND == ECHO_MINIAUDIO

#include <cstring>
#include <Engine/Core/System/Echo.h>
#include "EchoGlobals.h"
#include <miniaudio.h>

#include "Engine/Util/Log.h"

namespace Echo
{
    AudioSoundHandle CreateSound(const SoundDesc &desc)
    {
        if (!loadedSoundNames.contains(desc.name))
        {
            AudioSound newSound{};
            newSound.name = desc.name;
            newSound.channelLayout = desc.channelLayout;
            newSound.sampleRate = desc.sampleRate;
            newSound.sizeBytes = desc.sizeBytes;

            // this might seem redundant, but this is just for security.
            newSound.soundData = wNewArr(byte, newSound.sizeBytes);
            memcpy(newSound.soundData, desc.audioBuffer, newSound.sizeBytes);
            newSound.isLoaded = true;

            loadedSounds.push_back(newSound);
            loadedSoundNames[desc.name] = loadedSounds.size();
            return loadedSounds.size();
        }

        auto& snd = loadedSounds[loadedSoundNames[desc.name] - 1];

        if (snd.isLoaded)
        {
            WEngine::WLog::SetConsoleWarning();
            WEngine::WLog::ConsoleLog("The sound is already loaded you baboon.");
            return loadedSoundNames[desc.name];
        }

        if (snd.sizeBytes != desc.sizeBytes)
        {
            WEngine::WLog::SetConsoleWarning();
            WEngine::WLog::ConsoleLog("Tried to wake up sound of same name, but size is different!");
            return 0;
        }

        snd.soundData = wNewArr(byte, snd.sizeBytes);
        memcpy(snd.soundData, desc.audioBuffer, snd.sizeBytes);
        snd.isLoaded = true;

        return loadedSoundNames[desc.name];
    }

    AudioStreamHandle CreateStream(const StreamDesc &desc)
    {

    }

    AudioBusHandle CreateBus(const BusDesc &desc)
    {
        AudioBus newBus{};
        newBus.name = desc.name;
        newBus.volume = desc.volume;
        newBus.pitch = desc.pitch;

        loadedBusses.push_back(newBus);
        return loadedBusses.size();
    }

    void DeleteSound(AudioSoundHandle sound)
    {
        if (sound == 0 || sound > loadedSounds.size())
        {
            WEngine::WLog::SetConsoleWarning();
            WEngine::WLog::ConsoleLog("Invalid sound handle, refusing delete sound!");
            return;
        }

        auto& snd = loadedSounds[sound - 1];
        snd.isLoaded = false;
        wFree(snd.soundData);
    }

    void DeleteStream(AudioSoundHandle sound)
    {

    }
}

#endif
