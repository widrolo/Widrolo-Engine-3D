#pragma once

#include <string>
#include "CommonTypes.h"
#include <SDL3/SDL.h>

namespace WEngine
{
	/**
 	 * Represents the type of audio clip.
 	 */
	enum class AudioType
	{
		None,		///< Will not play
		UISound,	///< Won't stream, is not local
		ShortSound,	///< Won't stream, is local
		LongSound,	///< Will stream, is local
		Music,		///< Will stream, is not local
	};

	/**
	 * Represents an audio clip with its type, buffer, length, format, channels and frequency.
	 */
	struct AudioClip
	{
		AudioType type;					///< The type of the audio clip
		uint8* audioBuf;				///< The audio data buffer
		uint32 audioLen;				///< Mystery variable
		SDL_AudioFormat format;			///< The audio format
		int channels;					///< The number of audio channels
		int freq;						///< The frequency of the audio clip
	};

	struct AudioPlayer
	{
		AudioClip* clip;				///< The audio clip selected
		uint64 currentSample;			///< The current sample position in the audio clip
		SDL_AudioStream* audioStream;	///< The audio stream for playback
		bool loop;						///< Indicates if the audio clip should loop
		bool isPlaying;					///< Indicates if the audio clip is currently playing
	};
}
