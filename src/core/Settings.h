#pragma once

#include <string>

struct SettingsData
{
	int soundVolume = 10; // 0-10
	int musicVolume = 10; // 0-10

	// Graphics fields will be added here later; comparison/revert keep working.

	bool operator==(const SettingsData& other) const = default;
};

class Settings
{
public:
	void Load(const std::string& path); // missing file: keep defaults
	void Save(const std::string& path);

	bool IsDirty() const { return !(current == saved); }
	void Revert() { current = saved; } // discard unsaved changes

	int GetSoundVolume() const { return current.soundVolume; }
	int GetMusicVolume() const { return current.musicVolume; }

	void SetSoundVolume(int value);
	void SetMusicVolume(int value);

private:
	SettingsData current;
	SettingsData saved;
};