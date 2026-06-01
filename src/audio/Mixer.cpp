#include "Mixer.h"

#include "core/Resources.h"

#include <SFML/Audio/Listener.hpp>
#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

namespace Audio
{
	Mixer::Mixer(Resources& resources)
		: resources(resources)
		, soundPlayer(resources)
	{}

	void Mixer::RegisterMusic(const std::string& name, const std::string& path, bool loop)
	{
		musicPlayer.Register(name, path, loop);
	}

	void Mixer::PlayMusic(const std::string& name)
	{
		musicPlayer.Play(name);
	}

	void Mixer::StopMusic()
	{
		musicPlayer.Stop();
	}

	void Mixer::SetMusicVolume(float volume)
	{
		musicPlayer.SetVolume(volume);
	}

	void Mixer::RegisterSound(const std::string& name, const std::string& bufferName, float volume)
	{
		soundPlayer.Register(name, bufferName, volume);
	}

	void Mixer::PlaySound(const std::string& name)
	{
		soundPlayer.Play(name);
	}

	void Mixer::PlaySoundAt(const std::string& name, sf::Vector2f position)
	{
		soundPlayer.PlayAt(name, position);
	}

	void Mixer::SetSoundVolume(float volume)
	{
		soundPlayer.SetVolume(volume);
	}

	void Mixer::SetListenerPosition(sf::Vector2f position)
	{
		sf::Listener::setPosition({ position.x, position.y, 0.0f });
	}

	void Mixer::SetSpatialAttenuation(float minDistance, float attenuation)
	{
		soundPlayer.SetSpatialAttenuation(minDistance, attenuation);
	}
	
	void Mixer::LoadFromFile(const std::string& path)
	{
		std::ifstream file(path);
		if (!file)
			throw std::runtime_error("Audio::Mixer: cannot open file '" + path + "'");

		nlohmann::json data;
		file >> data;

		if (data.contains("sounds"))
		{
			for (const auto& [name, soundData] : data["sounds"].items())
			{
				const std::string soundPath = soundData.at("path");
				const float volume = soundData.value("volume", 1.0f);

				resources.sounds.Load(name, soundPath);
				RegisterSound(name, name, volume);
			}
		}

		if (data.contains("music"))
		{
			for (const auto& [name, musicData] : data["music"].items())
			{
				const std::string musicPath = musicData.at("path");
				const bool loop = musicData.value("loop", true);

				RegisterMusic(name, musicPath, loop);
			}
		}
	}
}