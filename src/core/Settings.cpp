#include "Settings.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <stdexcept>

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
	current.vsync = value;
}

void Settings::SetShowFps(bool value)
{
	current.showFps = value;
}

void Settings::Load(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		saved = current;
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

	if (data.contains("graphics"))
	{
		const auto& graphics = data["graphics"];
		current.resolutionWidth = graphics.value("width", current.resolutionWidth);
		current.resolutionHeight = graphics.value("height", current.resolutionHeight);
		current.screenMode = ScreenModeFromString(graphics.value("screenMode", ScreenModeToString(current.screenMode)));
		current.vsync = graphics.value("vsync", current.vsync);
		current.showFps = graphics.value("showFps", current.showFps);
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
	data["graphics"]["vsync"] = current.vsync;
	data["graphics"]["showFps"] = current.showFps;

	std::ofstream file(path);
	if (!file.is_open())
		throw std::runtime_error("Settings: cannot write '" + path + "'");

	file << data.dump(1, '\t');

	saved = current;
}