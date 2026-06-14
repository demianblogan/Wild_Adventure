#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct Resources;

namespace Audio
{
	// Plays one-shot sound effects through a fixed pool of reusable "voices"
	// (sf::Sound channels), so overlapping effects don't cut each other off.
	// Looping sounds are tracked separately, since they play until stopped.
	class SoundPlayer
	{
	public:
		static constexpr std::size_t DEFAULT_VOICE_COUNT = 32; // max simultaneous effects

		SoundPlayer(Resources& resources, std::size_t voiceCount = DEFAULT_VOICE_COUNT);

		// A sound's name doubles as the key of its loaded buffer in Resources.
		void Register(const std::string& soundName, float volume = 1.0f);

		void Play(const std::string& soundName);

		void StartLoop(const std::string& soundName);
		void StopLoop(const std::string& soundName);

		void SetVolume(float volume);

	private:
		// One playback channel from the pool: the SFML sound plus the sound's own
		// volume, kept so master-volume changes can be re-applied while it plays.
		struct Voice
		{
			std::unique_ptr<sf::Sound> sound;
			float baseVolume = 1.0f;
		};

		Voice* FindFreeVoice();
		void ConfigureVoice(Voice& voice, const std::string& soundName, float volume);

		Resources& resources;

		// sf::Sound needs a buffer at construction, but the pool is built before any
		// real buffer exists; this empty one is the placeholder until Play swaps in
		// the real buffer.
		sf::SoundBuffer placeholderBuffer;

		std::vector<Voice> voices;
		std::unordered_map<std::string, float> registeredSounds; // name -> volume
		std::unordered_map<std::string, std::unique_ptr<sf::Sound>> activeLoops;
		float masterVolume = 1.0f;
	};
}