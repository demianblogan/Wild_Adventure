#include "Settings.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <stdexcept>

void Settings::SetSoundVolume(int value)
{
	soundVolume = std::clamp(value, 0, 10);
}

void Settings::SetMusicVolume(int value)
{
	musicVolume = std::clamp(value, 0, 10);
}

void Settings::Load(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
		return; // no settings yet: keep defaults, the file appears on first Save

	nlohmann::json data;
	file >> data;

	if (data.contains("audio"))
	{
		const auto& audio = data["audio"];
		SetSoundVolume(audio.value("sound", soundVolume));
		SetMusicVolume(audio.value("music", musicVolume));
	}
}

void Settings::Save(const std::string& path) const
{
	nlohmann::json data;
	data["audio"]["sound"] = soundVolume;
	data["audio"]["music"] = musicVolume;

	std::ofstream file(path);
	if (!file.is_open())
		throw std::runtime_error("Settings: cannot write '" + path + "'");

	file << data.dump(1, '\t');
}