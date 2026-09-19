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
			settings.SetLanguage(Language::Ukrainian);
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
		CHECK(reloaded.GetLanguage() == Language::Ukrainian);
		CHECK(reloaded.IsLanguageChosen());
		CHECK_FALSE(reloaded.IsDirty()); // Load must mark the freshly loaded state as saved
	}

	TEST_CASE("SetLanguage marks the language as chosen, and Load falls back to defaults for an old save file")
	{
		Settings settings;
		CHECK(settings.GetLanguage() == Language::English);
		CHECK_FALSE(settings.IsLanguageChosen());

		settings.SetLanguage(Language::Russian);
		CHECK(settings.GetLanguage() == Language::Russian);
		CHECK(settings.IsLanguageChosen());

		const TempDirectory dir;
		const std::string path = (dir.GetPath() / "settings.json").string();
		// A save file from before localization existed has no "localization" section at all.
		REQUIRE(SafeFileWrite::WriteFileAtomically(path, "{ \"audio\": { \"sound\": 10, \"music\": 10 } }"));

		Settings reloaded;
		reloaded.Load(path);

		CHECK(reloaded.GetLanguage() == Language::English);
		CHECK_FALSE(reloaded.IsLanguageChosen());
	}

	TEST_CASE("A failed Save leaves the settings dirty instead of reporting success")
	{
		const TempDirectory dir;
		// Save() creates missing parent directories on its own, so to force a
		// genuine failure, a plain file (not a directory) sits where the
		// parent directory needs to be -- create_directories can't replace it.
		const std::filesystem::path blocker = dir.GetPath() / "blocked";
		REQUIRE(SafeFileWrite::WriteFileAtomically(blocker, "not a directory"));
		const std::string path = (blocker / "settings.json").string();

		Settings settings;
		settings.SetSoundVolume(3);

		CHECK_FALSE(settings.Save(path));
		CHECK(settings.IsDirty()); // must not be mistaken for a successful save
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
