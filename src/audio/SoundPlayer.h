#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Vector2.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct Resources;

namespace Audio
{
	class SoundPlayer
	{
	public:
		struct SoundConfig
		{
			std::string bufferName;
			float volume = 1.0f;
		};

		SoundPlayer(Resources& resources, std::size_t poolSize = 32);

		void Register(const std::string& name, const std::string& bufferName, float volume = 1.0f);

		void Play(const std::string& name);
		void PlayAt(const std::string& name, sf::Vector2f position);

		void SetVolume(float volume);
		void SetSpatialAttenuation(float minDistance, float attenuation);

	private:
		struct SoundSlot
		{
			std::unique_ptr<sf::Sound> sound;
			float baseVolume = 1.0f;
		};

		SoundSlot* FindFreeSlot();
		void ConfigureSlot(SoundSlot& slot, const SoundConfig& config);

		Resources& resources;
		sf::SoundBuffer dummyBuffer;
		std::vector<SoundSlot> pool;
		std::unordered_map<std::string, SoundConfig> registeredSounds;
		float masterVolume = 1.0f;
		float spatialMinDistance = 100.0f;
		float spatialAttenuation = 1.0f;
	};
}