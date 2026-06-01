#pragma once

#include "audio/MusicPlayer.h"
#include "audio/SoundPlayer.h"

#include <SFML/System/Vector2.hpp>
	
#include <string>

struct Resources;

namespace Audio
{
	class Mixer
	{
	public:
		Mixer(Resources& resources);

		void RegisterMusic(const std::string& name, const std::string& path, bool loop = true);
		void PlayMusic(const std::string& name);
		void StopMusic();
		void SetMusicVolume(float volume);

		void RegisterSound(const std::string& name, const std::string& bufferName, float volume = 1.0f);
		void PlaySound(const std::string& name);
		void PlaySoundAt(const std::string& name, sf::Vector2f position);
		void SetSoundVolume(float volume);

		void SetListenerPosition(sf::Vector2f position);
		void SetSpatialAttenuation(float minDistance, float attenuation);

		void LoadFromFile(const std::string& path);

	private:
		Resources& resources;
		MusicPlayer musicPlayer;
		SoundPlayer soundPlayer;
	};
}