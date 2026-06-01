#include "SoundPlayer.h"

#include "core/Resources.h"

#include <algorithm>
#include <stdexcept>

namespace Audio
{
	SoundPlayer::SoundPlayer(Resources& resources, std::size_t poolSize)
		: resources(resources)
	{
		pool.reserve(poolSize);
		for (std::size_t i = 0; i < poolSize; i++)
			pool.push_back({ std::make_unique<sf::Sound>(dummyBuffer), 1.0f });
	}

	void SoundPlayer::Register(const std::string& name, const std::string& bufferName, float volume)
	{
		registeredSounds[name] = { bufferName, volume };
	}

	void SoundPlayer::Play(const std::string& name)
	{
		const auto it = registeredSounds.find(name);
		if (it == registeredSounds.end())
			throw std::runtime_error("Audio::SoundPlayer: sound '" + name + "' is not registered");

		SoundSlot* freeSlot = FindFreeSlot();
		if (freeSlot == nullptr)
			return;

		ConfigureSlot(*freeSlot, it->second);

		freeSlot->sound->setRelativeToListener(true);
		freeSlot->sound->setPosition({ 0.0f, 0.0f, 0.0f });
		freeSlot->sound->play();
	}

	void SoundPlayer::PlayAt(const std::string& name, sf::Vector2f position)
	{
		const auto it = registeredSounds.find(name);
		if (it == registeredSounds.end())
			throw std::runtime_error("Audio::SoundPlayer: sound '" + name + "' is not registered");

		SoundSlot* freeSlot = FindFreeSlot();
		if (freeSlot == nullptr)
			return;

		ConfigureSlot(*freeSlot, it->second);

		freeSlot->sound->setRelativeToListener(false);
		freeSlot->sound->setPosition({ position.x, position.y, 0.0f });
		freeSlot->sound->setMinDistance(spatialMinDistance);
		freeSlot->sound->setAttenuation(spatialAttenuation);
		freeSlot->sound->play();
	}

	void SoundPlayer::SetVolume(float volume)
	{
		masterVolume = std::clamp(volume, 0.0f, 1.0f);

		for (SoundSlot& slot : pool)
			if (slot.sound->getStatus() == sf::Sound::Status::Playing)
				slot.sound->setVolume(slot.baseVolume * masterVolume * 100.0f);
	}

	void SoundPlayer::SetSpatialAttenuation(float minDistance, float attenuation)
	{
		spatialMinDistance = minDistance;
		spatialAttenuation = attenuation;
	}

	SoundPlayer::SoundSlot* SoundPlayer::FindFreeSlot()
	{
		for (SoundSlot& slot : pool)
			if (slot.sound->getStatus() == sf::Sound::Status::Stopped)
				return &slot;

		return nullptr;
	}

	void SoundPlayer::ConfigureSlot(SoundSlot& slot, const SoundConfig& config)
	{
		const sf::SoundBuffer& buffer = resources.sounds.Get(config.bufferName);
		slot.sound->setBuffer(buffer);
		slot.baseVolume = config.volume;
		slot.sound->setVolume(slot.baseVolume * masterVolume * 100.0f);
	}
}