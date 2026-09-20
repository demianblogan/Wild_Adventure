#include "Settings.h"

#include "core/SafeFileWrite.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace
{
	std::string ScreenModeToString(ScreenMode mode)
	{
		switch (mode)
		{
		case ScreenMode::Fullscreen: return "fullscreen";
		case ScreenMode::Borderless: return "borderless";
		case ScreenMode::Window:     return "window";
		}
		return "borderless";
	}

	ScreenMode ScreenModeFromString(const std::string& text)
	{
		if (text == "fullscreen") return ScreenMode::Fullscreen;
		if (text == "window")     return ScreenMode::Window;
		return ScreenMode::Borderless;
	}
}

void Settings::SetSoundVolume(int value)
{
	current.soundVolume = std::clamp(value, 0, 10);
}

void Settings::SetMusicVolume(int value)
{
	current.musicVolume = std::clamp(value, 0, 10);
}

void Settings::SetResolution(int width, int height)
{
	current.resolutionWidth = width;
	current.resolutionHeight = height;
}

void Settings::SetScreenMode(ScreenMode mode)
{
	current.screenMode = mode;
}

void Settings::SetVsync(bool value)
{
	current.isVsyncEnabled = value;
}

void Settings::SetShowFps(bool value)
{
	current.isShowFpsEnabled = value;
}

void Settings::SetVibrationEnabled(bool value)
{
	current.isVibrationEnabled = value;
}

void Settings::SetLightbarEnabled(bool value)
{
	current.isLightbarEnabled = value;
}

void Settings::SetLanguage(Language value)
{
	current.language = value;
	current.isLanguageChosen = true;
}

void Settings::Load(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		saved = current;
		return;
	}

	try
	{
		nlohmann::json data;
		file >> data;

		if (data.contains("audio"))
		{
			const auto& audio = data["audio"];
			SetSoundVolume(audio.value("sound", current.soundVolume));
			SetMusicVolume(audio.value("music", current.musicVolume));
		}

		if (data.contains("graphics"))
		{
			const auto& graphics = data["graphics"];
			current.resolutionWidth = graphics.value("width", current.resolutionWidth);
			current.resolutionHeight = graphics.value("height", current.resolutionHeight);
			current.screenMode = ScreenModeFromString(graphics.value("screenMode", ScreenModeToString(current.screenMode)));
			current.isVsyncEnabled = graphics.value("vsync", current.isVsyncEnabled);
			current.isShowFpsEnabled = graphics.value("showFps", current.isShowFpsEnabled);
		}

		if (data.contains("gameplay"))
		{
			const auto& gameplay = data["gameplay"];
			current.isVibrationEnabled = gameplay.value("vibration", current.isVibrationEnabled);
			current.isLightbarEnabled = gameplay.value("lightbar", current.isLightbarEnabled);
		}

		if (data.contains("localization"))
		{
			const auto& localization = data["localization"];
			current.language = LanguageFromCode(localization.value("language", LanguageCode(current.language)));
			current.isLanguageChosen = localization.value("languageChosen", current.isLanguageChosen);
		}
	}
	catch (const nlohmann::json::exception&)
	{
		// A hand-edited or crash-truncated settings file must never take the
		// game down with it: fall back to defaults and keep the bad file
		// around (renamed aside) for inspection instead of silently
		// overwriting it.
		current = SettingsData();

		file.close();
		static_cast<void>(SafeFileWrite::PreserveCorruptFile(path));
	}

	saved = current;
}

bool Settings::Save(const std::string& path)
{
	nlohmann::json data;
	data["audio"]["sound"] = current.soundVolume;
	data["audio"]["music"] = current.musicVolume;

	data["graphics"]["width"] = current.resolutionWidth;
	data["graphics"]["height"] = current.resolutionHeight;
	data["graphics"]["screenMode"] = ScreenModeToString(current.screenMode);
	data["graphics"]["vsync"] = current.isVsyncEnabled;
	data["graphics"]["showFps"] = current.isShowFpsEnabled;

	data["gameplay"]["vibration"] = current.isVibrationEnabled;
	data["gameplay"]["lightbar"] = current.isLightbarEnabled;

	data["localization"]["language"] = LanguageCode(current.language);
	data["localization"]["languageChosen"] = current.isLanguageChosen;

	// path lives under %LOCALAPPDATA%, which may not have been created yet
	// (e.g. this player's first launch, or after clearing it by hand).
	std::error_code ignored;
	std::filesystem::create_directories(std::filesystem::path(path).parent_path(), ignored);

	if (!SafeFileWrite::WriteFileAtomically(path, data.dump(1, '\t')))
		return false;

	saved = current;
	return true;
}

void Settings::ResetAudioToDefaults()
{
	const SettingsData defaults;
	current.soundVolume = defaults.soundVolume;
	current.musicVolume = defaults.musicVolume;
}

void Settings::ResetGraphicsToDefaults()
{
	const SettingsData defaults;
	current.resolutionWidth = defaults.resolutionWidth;
	current.resolutionHeight = defaults.resolutionHeight;
	current.screenMode = defaults.screenMode;
	current.isVsyncEnabled = defaults.isVsyncEnabled;
	current.isShowFpsEnabled = defaults.isShowFpsEnabled;
}

void Settings::ResetGameplayToDefaults()
{
	const SettingsData defaults;
	current.isVibrationEnabled = defaults.isVibrationEnabled;
	current.isLightbarEnabled = defaults.isLightbarEnabled;
}
