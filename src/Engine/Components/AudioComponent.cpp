#include "AudioComponent.h"

#include <Engine/Types/CoreSystems.h>
#include <Engine/Core/World/Entity.h>
#include <Engine/Core/Handlers/AssetRepo.h>
#include <Engine/Core/Handlers/AudioHandler.h>

using namespace WEngine;

REGISTER_COMPONENT(AudioComponent)

AudioComponent::AudioComponent(Entity* e)
{
	COMP_SETUP("AudioComponent");
}

void AudioComponent::Awake(ComponentArgs ca)
{
	auto audioName = ca.GetStringFromParams("audioName");
	auto loop = ca.GetBoolFromParams("loop");
	auto playOnAwake = ca.GetBoolFromParams("playOnAwake");

	if (loop.HasValue())
		m_loop = loop.GetValue();
	else
		m_loop = false;

	if (playOnAwake.HasValue())
		m_playOnAwake = playOnAwake.GetValue();
	else
		m_playOnAwake = false;

	if (audioName.HasValue())
		SetNewAudioClip(audioName.GetValue());
	else
		m_player->clip = nullptr;
}

void AudioComponent::SetNewAudioClip(std::string name)
{
	AudioClipAssetMission mission{};
	mission.name = name;

	CoreSystems::GetAssetRepo()->GetAsset(mission);


	if (mission.clip == nullptr)
		return;

	m_currentClip = mission.clip;

	m_player = CoreSystems::GetAudioHandler()->NewAudioPlayer(mission.clip);
	if (m_player == nullptr)
		return;
	m_player->loop = m_loop;
	m_player->currentSample = 0;

	if (m_playOnAwake)
		CoreSystems::GetAudioHandler()->PlayAudioPlayer(m_player);

}

void AudioComponent::Play()
{
	// stupid
	if (m_player == nullptr)
		return;
	if (m_player->audioStream == nullptr)
		return;
	CoreSystems::GetAudioHandler()->PlayAudioPlayer(m_player);
}

void AudioComponent::Stop()
{
	if (m_player == nullptr)
		return;
	if (m_player->audioStream == nullptr)
		return;
	CoreSystems::GetAudioHandler()->PauseAudioPlayer(m_player);
}
