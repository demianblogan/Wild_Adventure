#include "doctest/doctest.h"

#include "core/SafeFileWrite.h"
#include "core/Settings.h"

#include "TempDirectory.h"

TEST_SUITE("Settings")
{
	TEST_CASE("SetSoundVolume and SetMusicVolume clamp to 0..10")
	{
		Settings settings;

		settings.SetSoundVolume(15);
		CHECK(settings.GetSoundVolume() == 10);

		settings.SetSoundVolume(-5);
		CHECK(settings.GetSoundVolume() == 0);

		settings.SetMusicVolume(7);
		CHECK(settings.GetMusicVolume() == 7);
	}

	TEST_CASE("IsDirty and Revert track unsaved changes")
	{
		Settings settings;
		CHECK_FALSE(settings.IsDirty());

		settings.SetSoundVolume(3);
		CHECK(settings.IsDirty());

		settings.Revert();
		CHECK_FALSE(settings.IsDirty());
		CHECK(settings.GetSoundVolume() == 10); // back to the default
	}

	TEST_CASE("ResetGraphicsToDefaults restores the factory resolution and mode")
	{
		Settings settings;
		settings.SetResolution(800, 600);
		settings.SetScreenMode(ScreenMode::Window);

		settings.ResetGraphicsToDefaults();

		CHECK(settings.GetResolutionWidth() == 1920);
		CHECK(settings.GetResolutionHeight() == 1080);
		CHECK(settings.GetScreenMode() == ScreenMode::Borderless);
	}

	TEST_CASE("Save and Load round-trip every field")
	{
		const TempDirectory dir;
		const std::string path = (dir.GetPath() / "settings.json").string();

		{
			Settings settings;
			settings.SetSoundVolume(4);
			settings.SetMusicVolume(9);
			settings.SetResolution(1280, 720);
			settings.SetScreenMode(ScreenMode::Window);
			settings.SetVsync(false);
			settings.Save(path);
		}

		Settings reloaded;
		reloaded.Load(path);

		CHECK(reloaded.GetSoundVolume() == 4);
		CHECK(reloaded.GetMusicVolume() == 9);
		CHECK(reloaded.GetResolutionWidth() == 1280);
		CHECK(reloaded.GetResolutionHeight() == 720);
		CHECK(reloaded.GetScreenMode() == ScreenMode::Window);
		CHECK_FALSE(reloaded.IsVsyncEnabled());
		CHECK_FALSE(reloaded.IsDirty()); // Load must mark the freshly loaded state as saved
	}

	TEST_CASE("Loading a corrupt settings file falls back to defaults and preserves the file")
	{
		const TempDirectory dir;
		const std::filesystem::path path = dir.GetPath() / "settings.json";

		REQUIRE(SafeFileWrite::WriteFileAtomically(path, "{ \"audio\": [ this is not json"));

		Settings settings;
		settings.Load(path.string());

		CHECK(settings.GetSoundVolume() == 10);
		CHECK(settings.GetResolutionWidth() == 1920);
		CHECK_FALSE(std::filesystem::exists(path));
		CHECK(std::filesystem::exists(dir.GetPath() / "settings.json.corrupt"));
	}
}
