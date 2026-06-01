#pragma once

#include <SFML/Audio/Music.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace Audio
{
	class MusicPlayer
	{
	public:
		struct MusicConfig
		{
			std::string path;
			bool loop = true;
		};

		void Register(const std::string& name, const std::string& path, bool loop = true);

		void Play(const std::string& name);
		void Stop();

		void SetVolume(float volume);

	private:
		void ApplyVolume();

		std::unordered_map<std::string, MusicConfig> registeredMusics;
		std::unique_ptr<sf::Music> currentMusic;
		float volume = 1.0f;
	};
}