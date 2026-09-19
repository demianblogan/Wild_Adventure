#include "Settings.h"

#include "core/SafeFileWrite.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>

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

void Settings::Save(const std::string& path)
{
	nlohmann::json data;
	data["audio"]["sound"] = current.soundVolume;
	data["audio"]["music"] = current.musicVolume;

	data["graphics"]["width"] = current.resolutionWidth;
	data["graphics"]["height"] = current.resolutionHeight;
	data["graphics"]["screenMode"] = ScreenModeToString(current.screenMode);
	data["graphics"]["vsync"] = current.isVsyncEnabled;
	data["graphics"]["showFps"] = current.isShowFpsEnabled;

	static_cast<void>(SafeFileWrite::WriteFileAtomically(path, data.dump(1, '\t')));

	saved = current;
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
