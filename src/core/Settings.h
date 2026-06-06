#pragma once

#include <string>

class Settings
{
public:
	void Load(const std::string& path); // missing file: keep defaults
	void Save(const std::string& path) const;

	int GetSoundVolume() const { return soundVolume; } // 0-10
	int GetMusicVolume() const { return musicVolume; } // 0-10

	void SetSoundVolume(int value);
	void SetMusicVolume(int value);

private:
	int soundVolume = 10;
	int musicVolume = 10;
};