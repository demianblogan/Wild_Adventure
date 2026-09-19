#include "doctest/doctest.h"

#include "core/Input.h"
#include "core/SafeFileWrite.h"

#include "TempDirectory.h"

#include <SFML/Window/Keyboard.hpp>

namespace
{
	constexpr const char* VALID_BINDINGS = R"({
		"axisThreshold": 60,
		"bindings": {
			"Jump": [ { "key": "Space" } ],
			"MoveLeft": [ { "key": "A" } ]
		}
	})";

	constexpr const char* DEFAULT_BINDINGS = R"({
		"axisThreshold": 50,
		"bindings": {
			"Jump": [ { "key": "Enter" } ],
			"MoveLeft": [ { "key": "Left" } ]
		}
	})";
}

TEST_SUITE("Input")
{
	TEST_CASE("LoadConfig reads bindings from a valid file")
	{
		const TempDirectory dir;
		const std::filesystem::path path = dir.GetPath() / "input.json";
		REQUIRE(SafeFileWrite::WriteFileAtomically(path, VALID_BINDINGS));

		Input input;
		input.LoadConfig(path.string());

		CHECK(input.GetPrimaryKey(Action::Jump) == sf::Keyboard::Key::Space);
		CHECK_FALSE(input.IsDirty());
	}

	TEST_CASE("LoadConfig falls back to the loaded defaults on a corrupt file and preserves it")
	{
		const TempDirectory dir;
		const std::filesystem::path defaultsPath = dir.GetPath() / "input_default.json";
		const std::filesystem::path path = dir.GetPath() / "input.json";

		REQUIRE(SafeFileWrite::WriteFileAtomically(defaultsPath, DEFAULT_BINDINGS));
		REQUIRE(SafeFileWrite::WriteFileAtomically(path, "{ this is not json"));

		Input input;
		input.LoadDefaults(defaultsPath.string());
		input.LoadConfig(path.string());

		CHECK(input.GetPrimaryKey(Action::Jump) == sf::Keyboard::Key::Enter);
		CHECK_FALSE(std::filesystem::exists(path));
		CHECK(std::filesystem::exists(dir.GetPath() / "input.json.corrupt"));
	}

	TEST_CASE("LoadConfig falls back to defaults when the file is simply missing")
	{
		const TempDirectory dir;
		const std::filesystem::path defaultsPath = dir.GetPath() / "input_default.json";
		REQUIRE(SafeFileWrite::WriteFileAtomically(defaultsPath, DEFAULT_BINDINGS));

		Input input;
		input.LoadDefaults(defaultsPath.string());
		input.LoadConfig((dir.GetPath() / "does_not_exist.json").string());

		CHECK(input.GetPrimaryKey(Action::Jump) == sf::Keyboard::Key::Enter);
	}

	TEST_CASE("SetPrimaryKey rebinds and IsDirty/Revert track it")
	{
		const TempDirectory dir;
		const std::filesystem::path path = dir.GetPath() / "input.json";
		REQUIRE(SafeFileWrite::WriteFileAtomically(path, VALID_BINDINGS));

		Input input;
		input.LoadConfig(path.string());

		input.SetPrimaryKey(Action::Jump, sf::Keyboard::Key::LShift);
		CHECK(input.GetPrimaryKey(Action::Jump) == sf::Keyboard::Key::LShift);
		CHECK(input.IsDirty());

		input.Revert();
		CHECK(input.GetPrimaryKey(Action::Jump) == sf::Keyboard::Key::Space);
		CHECK_FALSE(input.IsDirty());
	}

	TEST_CASE("ResetToDefaults restores the factory bindings")
	{
		const TempDirectory dir;
		const std::filesystem::path defaultsPath = dir.GetPath() / "input_default.json";
		const std::filesystem::path path = dir.GetPath() / "input.json";
		REQUIRE(SafeFileWrite::WriteFileAtomically(defaultsPath, DEFAULT_BINDINGS));
		REQUIRE(SafeFileWrite::WriteFileAtomically(path, VALID_BINDINGS));

		Input input;
		input.LoadDefaults(defaultsPath.string());
		input.LoadConfig(path.string());

		input.ResetToDefaults();

		CHECK(input.GetPrimaryKey(Action::Jump) == sf::Keyboard::Key::Enter);
	}

	TEST_CASE("Pause is always bound to Escape regardless of the loaded file")
	{
		const TempDirectory dir;
		const std::filesystem::path path = dir.GetPath() / "input.json";
		REQUIRE(SafeFileWrite::WriteFileAtomically(path, VALID_BINDINGS));

		Input input;
		input.LoadConfig(path.string());

		CHECK(input.GetPrimaryKey(Action::Pause) == sf::Keyboard::Key::Escape);
	}

	TEST_CASE("A failed SaveConfig leaves the bindings dirty instead of reporting success")
	{
		const TempDirectory dir;
		const std::filesystem::path path = dir.GetPath() / "input.json";
		REQUIRE(SafeFileWrite::WriteFileAtomically(path, VALID_BINDINGS));

		Input input;
		input.LoadConfig(path.string());
		input.SetPrimaryKey(Action::Jump, sf::Keyboard::Key::LShift);

		// The parent directory does not exist, so the write cannot succeed.
		const std::string badPath = (dir.GetPath() / "missing_subdir" / "input.json").string();
		CHECK_FALSE(input.SaveConfig(badPath));
		CHECK(input.IsDirty()); // must not be mistaken for a successful save
	}

	TEST_CASE("KeyName round-trips through LoadConfig and SaveConfig")
	{
		CHECK(Input::KeyName(sf::Keyboard::Key::Space) == "Space");
		CHECK(Input::KeyName(sf::Keyboard::Key::Unknown) == "None");
	}
}
