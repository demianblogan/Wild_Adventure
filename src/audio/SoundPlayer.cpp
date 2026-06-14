#include "SoundPlayer.h"

#include "core/Resources.h"

#include <algorithm>
#include <stdexcept>

namespace Audio
{
	SoundPlayer::SoundPlayer(Resources& resources, std::size_t voiceCount)
		: resources(resources)
	{
		voices.reserve(voiceCount);
		for (std::size_t i = 0; i < voiceCount; i++)
			voices.push_back({ std::make_unique<sf::Sound>(placeholderBuffer), 1.0f });
	}

	void SoundPlayer::Register(const std::string& soundName, float volume)
	{
		registeredSounds[soundName] = volume;
	}

	void SoundPlayer::Play(const std::string& soundName)
	{
		const auto entry = registeredSounds.find(soundName);
		if (entry == registeredSounds.end())
			throw std::runtime_error("Audio::SoundPlayer: sound '" + soundName + "' is not registered");

		Voice* freeVoice = FindFreeVoice();
		if (freeVoice == nullptr)
			return;

		ConfigureVoice(*freeVoice, soundName, entry->second);
		freeVoice->sound->play();
	}

	void SoundPlayer::StartLoop(const std::string& soundName)
	{
		const auto entry = registeredSounds.find(soundName);
		if (entry == registeredSounds.end())
			throw std::runtime_error("Audio::SoundPlayer: sound '" + soundName + "' is not registered");

		const auto existingLoop = activeLoops.find(soundName);
		if (existingLoop != activeLoops.end() && existingLoop->second->getStatus() == sf::Sound::Status::Playing)
			return; // already looping

		const float volume = entry->second;
		const sf::SoundBuffer& buffer = resources.sounds.Get(soundName);

		auto sound = std::make_unique<sf::Sound>(buffer);
		sound->setLooping(true);
		sound->setVolume(volume * masterVolume * 100.0f);
		sound->play();

		activeLoops[soundName] = std::move(sound);
	}

	void SoundPlayer::StopLoop(const std::string& soundName)
	{
		const auto existingLoop = activeLoops.find(soundName);
		if (existingLoop == activeLoops.end())
			return;

		existingLoop->second->stop();
		activeLoops.erase(existingLoop);
	}

	void SoundPlayer::SetVolume(float volume)
	{
		masterVolume = std::clamp(volume, 0.0f, 1.0f);

		for (Voice& voice : voices)
			if (voice.sound->getStatus() == sf::Sound::Status::Playing)
				voice.sound->setVolume(voice.baseVolume * masterVolume * 100.0f);

		for (auto& [soundName, sound] : activeLoops)
		{
			const auto entry = registeredSounds.find(soundName);
			const float baseVolume = (entry != registeredSounds.end()) ? entry->second : 1.0f;
			sound->setVolume(baseVolume * masterVolume * 100.0f);
		}
	}

	SoundPlayer::Voice* SoundPlayer::FindFreeVoice()
	{
		for (Voice& voice : voices)
			if (voice.sound->getStatus() == sf::Sound::Status::Stopped)
				return &voice;

		return nullptr;
	}

	void SoundPlayer::ConfigureVoice(Voice& voice, const std::string& soundName, float volume)
	{
		const sf::SoundBuffer& buffer = resources.sounds.Get(soundName);
		voice.sound->setBuffer(buffer);
		voice.baseVolume = volume;
		voice.sound->setVolume(volume * masterVolume * 100.0f);
	}
}