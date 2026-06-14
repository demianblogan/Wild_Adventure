#pragma once

#include "audio/MusicPlayer.h"
#include "audio/SoundPlayer.h"

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

		void RegisterSound(const std::string& name, float volume = 1.0f);
		void PlaySound(const std::string& name);

		void StartLoop(const std::string& name);
		void StopLoop(const std::string& name);

		void SetSoundVolume(float volume);

		void LoadFromFile(const std::string& path);

	private:
		Resources& resources;
		MusicPlayer musicPlayer;
		SoundPlayer soundPlayer;
	};
}