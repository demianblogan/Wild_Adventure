#include "Settings.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <stdexcept>

void Settings::SetSoundVolume(int value)
{
	current.soundVolume = std::clamp(value, 0, 10);
}

void Settings::SetMusicVolume(int value)
{
	current.musicVolume = std::clamp(value, 0, 10);
}

void Settings::Load(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		saved = current; // no file yet: defaults are the "saved" baseline
		return;
	}

	nlohmann::json data;
	file >> data;

	if (data.contains("audio"))
	{
		const auto& audio = data["audio"];
		SetSoundVolume(audio.value("sound", current.soundVolume));
		SetMusicVolume(audio.value("music", current.musicVolume));
	}

	saved = current;
}

void Settings::Save(const std::string& path)
{
	nlohmann::json data;
	data["audio"]["sound"] = current.soundVolume;
	data["audio"]["music"] = current.musicVolume;

	std::ofstream file(path);
	if (!file.is_open())
		throw std::runtime_error("Settings: cannot write '" + path + "'");

	file << data.dump(1, '\t');

	saved = current; // current state is now the saved baseline
}