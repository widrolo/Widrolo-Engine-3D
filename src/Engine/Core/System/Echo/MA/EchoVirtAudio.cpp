#if ECHO_BACKEND == ECHO_MINIAUDIO

#include <Engine/Core/System/Echo.h>
#include <miniaudio.h>

#include "EchoGlobals.h"
#include "Engine/Util/Log.h"
#include "Helpers/Types.h"

namespace Echo
{
    VirtualAudioHandle CreateAudio(AudioSoundHandle sound)
    {
        VirtualAudio virt{};
        virt.type = VirtualAudioType::Static;
        virt.data.sound = sound;
        virt.volume = 1.0;
        virt.pitch = 1.0;
        virt.parent = nullptr; // crippled orphan by default. No nice guy stuff by default.

        virtualAudio.push_back(virt);
        return virtualAudio.size();
    }

    VirtualAudioHandle CreateAudio(AudioStreamHandle stream)
    {

    }

    VirtualAudioHandle SwapAudio(VirtualAudioHandle audio, AudioSoundHandle sound)
    {

    }

    VirtualAudioHandle SwapAudio(VirtualAudioHandle audio, AudioStreamHandle stream)
    {

    }

    void ConnectAudio(VirtualAudioHandle audio, AudioBusHandle busHandle)
    {
        if (audio == 0 || audio > virtualAudio.size())
        {
            WEngine::WLog::SetConsoleWarning();
            WEngine::WLog::ConsoleLog("Invalid virtual audio handle, refusing to connect to bus!");
            return;
        }

        if (busHandle > loadedBusses.size())
        {
            WEngine::WLog::SetConsoleWarning();
            WEngine::WLog::ConsoleLog("Invalid bus handle, refusing to connect to bus!");
            return;
        }

        auto& snd = virtualAudio[audio - 1];
        auto& bus = masterBus;
        if (busHandle != 0)
            bus = loadedBusses[busHandle - 1];

        // I already know that i will pull this one day
        if (snd.parent == &bus)
        {
            WEngine::WLog::SetConsoleWarning();
            WEngine::WLog::ConsoleLog("Virtual audio already connected to bus!");
            return;
        }

        if (snd.parent != nullptr)
            std::erase(snd.parent->children, &snd); // oh boy, lets hope i picked the right stl erase function today

        bus.children.push_back(&snd);
        snd.parent = &bus;
    }

    void SetAudioLoop(VirtualAudioHandle audio, bool loop)
    {
        if (audio == 0 || audio > virtualAudio.size())
        {
            WEngine::WLog::SetConsoleWarning();
            WEngine::WLog::ConsoleLog("Invalid virtual audio handle, refusing to set loop!");
            return;
        }

        auto& snd = virtualAudio[audio - 1];
        snd.isLooping = loop;
    }

    void SetAudioVolume(VirtualAudioHandle audio, float32 volume)
    {
        if (audio == 0 || audio > virtualAudio.size())
        {
            WEngine::WLog::SetConsoleWarning();
            WEngine::WLog::ConsoleLog("Invalid virtual audio handle, refusing to set volume!");
            return;
        }

        auto& snd = virtualAudio[audio - 1];
        snd.volume = volume;
    }

    void SetAudioPitch(VirtualAudioHandle audio, float32 pitch)
    {
        if (audio == 0 || audio > virtualAudio.size())
        {
            WEngine::WLog::SetConsoleWarning();
            WEngine::WLog::ConsoleLog("Invalid virtual audio handle, refusing to set pitch!");
            return;
        }

        auto& snd = virtualAudio[audio - 1];
        snd.pitch = pitch;
    }
}

#endif
