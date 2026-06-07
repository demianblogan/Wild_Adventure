#pragma once

#include <string>

enum class ScreenMode
{
	Fullscreen,
	Borderless,
	Window
};

struct SettingsData
{
	int soundVolume = 10; // 0-10
	int musicVolume = 10; // 0-10

	int resolutionWidth = 1920;
	int resolutionHeight = 1080;
	ScreenMode screenMode = ScreenMode::Borderless;
	bool vsync = true;
	bool showFps = false;

	bool operator==(const SettingsData& other) const = default;
};

class Settings
{
public:
	void Load(const std::string& path);
	void Save(const std::string& path);

	bool IsDirty() const { return !(current == saved); }
	void Revert() { current = saved; }

	int GetSoundVolume() const { return current.soundVolume; }
	int GetMusicVolume() const { return current.musicVolume; }
	void SetSoundVolume(int value);
	void SetMusicVolume(int value);

	int GetResolutionWidth() const { return current.resolutionWidth; }
	int GetResolutionHeight() const { return current.resolutionHeight; }
	ScreenMode GetScreenMode() const { return current.screenMode; }
	bool GetVsync() const { return current.vsync; }
	bool GetShowFps() const { return current.showFps; }

	void SetResolution(int width, int height);
	void SetScreenMode(ScreenMode mode);
	void SetVsync(bool value);
	void SetShowFps(bool value);

private:
	SettingsData current;
	SettingsData saved;
};