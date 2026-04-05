#pragma once

#include <Engine/Components/Component.h>
#include <Engine/Types/Audio.h>
#include <Engine/Types/SpawnArgs.h>

namespace WEngine
{
	class AudioComponent : public Component
	{
	public:
		AudioComponent(Entity* e);

	private:
		AudioPlayer* m_player;
		AudioClip* m_currentClip;

		bool m_loop;
		bool m_playOnAwake;

	public:
		void Awake(ComponentArgs ca) override;

		void SetNewAudioClip(std::string name);
		void Play();
		void Stop();

		COMP_HASH(0xf9fb8578273622be)
	};
}



