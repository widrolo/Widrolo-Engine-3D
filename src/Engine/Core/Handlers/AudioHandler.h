#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <Engine/WTL/vector.h>

namespace WEngine
{
	enum class AudioType;
	struct AudioClip;
	struct AudioPlayer;
	/**
	 * The AudioHandler manages the creation, swapping, and playing of audio clips using audio players.
	 */
	class AudioHandler
	{
	public:
		AudioHandler();

	private:
		wtl::vector<AudioPlayer*> m_players;

	public:
		/**
		 * Does nothing for now.
		 */
		void AudioTick();
		/**
		 * Creates a new audio player with the specified audio clip.
		 * @param clip The audio clip to be played. If nullptr, no player is created and nullptr is returned.
		 * @return A pointer to the newly created audio player, or nullptr if an error occurred during creation.
		 */
		AudioPlayer* NewAudioPlayer(AudioClip* clip);
		/**
		 * Swaps the audio clip of an existing audio player with a new one.
		 * @param player The audio player whose clip is to be swapped.
		 * @param newClip The new audio clip to be assigned to the player.
		 */
		void SwapAudioClip(AudioPlayer* player, AudioClip* newClip);
		/**
		 * Plays an audio player. If the player is already playing, this function does nothing.
		 * @param player The audio player to be played. If nullptr, the function does nothing.
		 */
		void PlayAudioPlayer(AudioPlayer* player);
		/**
		 * Pauses an audio player. If the player is already paused, this function does nothing.
		 * @param player The audio player to be paused. If nullptr, the function does nothing.
		 */
		void PauseAudioPlayer(AudioPlayer* player);
	};
}


