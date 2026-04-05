#include "AudioHandler.h"

#include <Engine/Util/Log.h>
#include <Engine/Types/Audio.h>

#include <Engine/Types/CoreSystems.h>


using namespace WEngine;

void AudioStreamCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
{
	auto* player = static_cast<AudioPlayer *>(userdata);
	const AudioClip* clip = player->clip;
	if (clip == nullptr)
		return;
	if (clip->audioBuf == nullptr)
		return;

	const uint32_t remaining = (clip->audioLen > player->currentSample)
		? (clip->audioLen - player->currentSample) : 0;

	if (remaining == 0)
		return;

	const int toCopy = (additional_amount < (int)remaining) ? additional_amount : (int)remaining;

	if (toCopy > 0)
	{
		SDL_PutAudioStreamData(stream, clip->audioBuf + player->currentSample, toCopy);
		player->currentSample += toCopy;
	}

	if (player->currentSample >= clip->audioLen)
	{
		if (player->loop)
			player->currentSample = 0;
	}
}

AudioHandler::AudioHandler()
{
	
}

void AudioHandler::AudioTick()
{
}

AudioPlayer* AudioHandler::NewAudioPlayer(AudioClip* clip)
{
	if (clip == nullptr)
		return nullptr;

	auto* player = new AudioPlayer();
	player->clip = clip;

	SDL_AudioSpec spec{};
	spec.channels = clip->channels;
	spec.format = clip->format;
	spec.freq = clip->freq;

	const auto stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, AudioStreamCallback, player);

	if (!stream)
	{
		WLog::SetConsoleError();
		WLog::ConsoleLog(std::format("Couldnt open audio stream: {}", SDL_GetError()));
		return nullptr;
	}
	

	player->audioStream = stream;

	m_players.push_back(player);

	return player;
}

void AudioHandler::SwapAudioClip(AudioPlayer* player, AudioClip* newClip)
{
	SDL_ClearAudioStream(player->audioStream);
	player->clip = newClip;
	player->currentSample = 0;
}

void AudioHandler::PlayAudioPlayer(AudioPlayer* player)
{
	if (player == nullptr)
		return;
	if (player->isPlaying == true)
		return;
	SDL_ResumeAudioStreamDevice(player->audioStream);
	player->isPlaying = true;
}

void AudioHandler::PauseAudioPlayer(AudioPlayer* player)
{
	if (player == nullptr)
		return;

	if (player->isPlaying == false)
		return;

	SDL_PauseAudioStreamDevice(player->audioStream);
	player->isPlaying = false;
}
