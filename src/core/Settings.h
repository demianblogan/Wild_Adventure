#pragma once

#include "localization/Language.h"

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
	bool isVsyncEnabled = true;
	bool isShowFpsEnabled = false;

	bool isVibrationEnabled = true;
	bool isLightbarEnabled = true;
	bool isLowHealthVignetteEnabled = true;
	bool isHitStopEnabled = true;
	bool isCameraShakeEnabled = true;

	Language language = Language::English;
	bool isLanguageChosen = false; // false until the first-run language picker has been confirmed

	bool operator==(const SettingsData& other) const = default;
};

class Settings
{
public:
	void Load(const std::string& path);

	// Returns false if the write failed (e.g. disk full, file locked); the
	// in-memory state then stays dirty so IsDirty() keeps reporting unsaved
	// changes instead of the caller believing the save went through.
	bool Save(const std::string& path);

	bool IsDirty() const { return !(current == saved); }
	void Revert() { current = saved; }

	void ResetAudioToDefaults();
	void ResetGraphicsToDefaults();
	void ResetGameplayToDefaults();

	int GetSoundVolume() const { return current.soundVolume; }
	int GetMusicVolume() const { return current.musicVolume; }
	void SetSoundVolume(int value);
	void SetMusicVolume(int value);

	int GetResolutionWidth() const { return current.resolutionWidth; }
	int GetResolutionHeight() const { return current.resolutionHeight; }
	ScreenMode GetScreenMode() const { return current.screenMode; }
	bool IsVsyncEnabled() const { return current.isVsyncEnabled; }
	bool IsShowFpsEnabled() const { return current.isShowFpsEnabled; }

	bool IsVibrationEnabled() const { return current.isVibrationEnabled; }
	bool IsLightbarEnabled() const { return current.isLightbarEnabled; }
	void SetVibrationEnabled(bool value);
	void SetLightbarEnabled(bool value);

	bool IsLowHealthVignetteEnabled() const { return current.isLowHealthVignetteEnabled; }
	bool IsHitStopEnabled() const { return current.isHitStopEnabled; }
	bool IsCameraShakeEnabled() const { return current.isCameraShakeEnabled; }
	void SetLowHealthVignetteEnabled(bool value);
	void SetHitStopEnabled(bool value);
	void SetCameraShakeEnabled(bool value);

	Language GetLanguage() const { return current.language; }
	bool IsLanguageChosen() const { return current.isLanguageChosen; }

	void SetResolution(int width, int height);
	void SetScreenMode(ScreenMode mode);
	void SetVsync(bool value);
	void SetShowFps(bool value);
	void SetLanguage(Language value); // also marks the language as chosen



private:
	SettingsData current;
	SettingsData saved;
};