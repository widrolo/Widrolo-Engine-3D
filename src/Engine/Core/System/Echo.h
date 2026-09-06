#pragma once

// This file does not contain LLM generated documentation

#include <Engine/Types/Audio/Echo/Handles.h>
#include <Engine/Types/Audio/Echo/Init.h>
#include <Engine/Types/Audio/Echo/Resource.h>

namespace Echo
{
    bool Init(const InitDesc& desc);
    void Shutdown();

    // --------------------------------------- Resources ---------------------------------------
    // ------- Creation --------
    AudioSoundHandle CreateSound(const SoundDesc& desc);
    AudioStreamHandle CreateStream(const StreamDesc& desc);
    AudioBusHandle CreateBus(const BusDesc& desc); // Handle 0 is master bus

    bool IsSoundLoaded(AudioSoundHandle handle);
    bool IsSoundLoaded(const std::string& name);
    bool IsStreamLoaded(AudioStreamHandle handle);
    bool IsStreamLoaded(const std::string& name);

    // ------- Deletion --------
    // note that Echo remembers sounds by name; even if deleted, the handle will
    // work later if a sound is loaded under the same name.
    void DeleteSound(AudioSoundHandle sound);
    // this pretty much just hibernates and rewinds the stream, the handle will
    // work later if a stream is created under the same filepath.
    void DeleteStream(AudioSoundHandle sound);

    // ---------------------------------------- Virtual ----------------------------------------
    VirtualAudioHandle CreateAudio(AudioSoundHandle sound);
    VirtualAudioHandle CreateAudio(AudioStreamHandle stream);

    VirtualAudioHandle SwapAudio(VirtualAudioHandle audio, AudioSoundHandle sound);
    VirtualAudioHandle SwapAudio(VirtualAudioHandle audio, AudioStreamHandle stream);

    void ConnectAudio(VirtualAudioHandle audio, AudioBusHandle busHandle);
    void SetAudioLoop(VirtualAudioHandle audio, bool loop);
    void SetAudioVolume(VirtualAudioHandle audio, float32 volume);
    void SetAudioPitch(VirtualAudioHandle audio, float32 pitch);

    // --------------------------------------- Playback ----------------------------------------
    void Play(VirtualAudioHandle audio);
    void Pause(VirtualAudioHandle audio);
    void Rewind(VirtualAudioHandle audio);
    void Rewind(VirtualAudioHandle audio, float64 position); // position is the % along the track

    bool IsPlaying(VirtualAudioHandle audio);
    float64 GetPlaybackPosition(VirtualAudioHandle audio); // in %
    float64 GetTrackLength(VirtualAudioHandle audio); // in seconds

    // ------------------------------------- Bus Settings --------------------------------------
    void BusVolume(AudioBusHandle bus, float32 volume);
    void BusPitch(AudioBusHandle bus, float32 pitch);

    // ----------------------------------------- Stats -----------------------------------------

}