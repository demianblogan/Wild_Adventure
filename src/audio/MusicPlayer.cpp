#include "MusicPlayer.h"

#include <algorithm>
#include <stdexcept>

namespace Audio
{
	void MusicPlayer::Register(const std::string& name, const std::string& path, bool loop)
	{
		registeredMusics[name] = { path, loop };
	}

	void MusicPlayer::Play(const std::string& name)
	{
		const auto iter = registeredMusics.find(name);
		if (iter == registeredMusics.end())
			throw std::runtime_error("MusicPlayer: music '" + name + "' is not registered");

		if (currentMusic)
			currentMusic->stop();

		currentMusic = std::make_unique<sf::Music>();
		if (!currentMusic->openFromFile(iter->second.path))
			throw std::runtime_error("MusicPlayer: cannot open music file '" + iter->second.path + "'");

		currentMusic->setLooping(iter->second.loop);
		ApplyVolume();
		currentMusic->play();
	}

	void MusicPlayer::Stop()
	{
		if (currentMusic)
		{
			currentMusic->stop();
			currentMusic.reset();
		}
	}

	void MusicPlayer::SetVolume(float newVolume)
	{
		volume = std::clamp(newVolume, 0.0f, 1.0f);
		ApplyVolume();
	}

	void MusicPlayer::ApplyVolume()
	{
		if (currentMusic)
			currentMusic->setVolume(volume * 100.0f);
	}
}