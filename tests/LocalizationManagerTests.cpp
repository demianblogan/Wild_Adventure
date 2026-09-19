#include "doctest/doctest.h"

#include "localization/LocalizationManager.h"

#include "TempDirectory.h"

#include <fstream>

namespace
{
	void WriteCatalog(const std::filesystem::path& path, const std::string& contents)
	{
		std::ofstream file(path);
		file << contents;
	}
}

TEST_SUITE("LocalizationManager")
{
	TEST_CASE("GetText prefers the active language's value over English's")
	{
		const TempDirectory dir;
		WriteCatalog(dir.GetPath() / "en.txt", "greeting.hello = Hello\n");
		WriteCatalog(dir.GetPath() / "de.txt", "greeting.hello = Hallo\n");

		LocalizationManager localization(dir.GetPath().string() + "/");
		localization.SetLanguage(Language::German);

		CHECK(localization.GetText("greeting.hello") == "Hallo");
	}

	TEST_CASE("GetText falls back to English when the active language's catalog is missing a key")
	{
		const TempDirectory dir;
		WriteCatalog(dir.GetPath() / "en.txt", "greeting.hello = Hello\n");
		WriteCatalog(dir.GetPath() / "de.txt", "# no greeting.hello here\n");

		LocalizationManager localization(dir.GetPath().string() + "/");
		localization.SetLanguage(Language::German);

		CHECK(localization.GetText("greeting.hello") == "Hello");
	}

	TEST_CASE("GetText returns the key itself when it is missing from every catalog")
	{
		const TempDirectory dir;
		WriteCatalog(dir.GetPath() / "en.txt", "greeting.hello = Hello\n");

		LocalizationManager localization(dir.GetPath().string() + "/");
		localization.SetLanguage(Language::English);

		CHECK(localization.GetText("greeting.missing") == "greeting.missing");
	}

	TEST_CASE("FormatText replaces a token placeholder in the resolved text")
	{
		const TempDirectory dir;
		WriteCatalog(dir.GetPath() / "en.txt", "score.line = Score: {score}\n");

		LocalizationManager localization(dir.GetPath().string() + "/");
		localization.SetLanguage(Language::English);

		CHECK(localization.FormatText("score.line", "score", "42") == "Score: 42");
	}

	TEST_CASE("SetLanguage increases Revision every time it is called")
	{
		const TempDirectory dir;
		WriteCatalog(dir.GetPath() / "en.txt", "");
		WriteCatalog(dir.GetPath() / "de.txt", "");

		LocalizationManager localization(dir.GetPath().string() + "/");
		const int initial = localization.Revision();

		localization.SetLanguage(Language::German);
		CHECK(localization.Revision() == initial + 1);

		localization.SetLanguage(Language::English);
		CHECK(localization.Revision() == initial + 2);
	}

	TEST_CASE("A \\n escape in a catalog value becomes a real line break")
	{
		const TempDirectory dir;
		WriteCatalog(dir.GetPath() / "en.txt", "dialog.body = Line one\\nLine two\n");

		LocalizationManager localization(dir.GetPath().string() + "/");
		localization.SetLanguage(Language::English);

		CHECK(localization.GetText("dialog.body") == "Line one\nLine two");
	}

	TEST_CASE("GetLanguageDisplayName and GetLanguagePickerPrompt read the code-keyed files directly")
	{
		const TempDirectory dir;
		WriteCatalog(dir.GetPath() / "language_names.txt", "en = English\nde = Deutsch\n");
		WriteCatalog(dir.GetPath() / "language_picker_prompt.txt", "en = Choose your language\nde = Wahle deine Sprache\n");

		const LocalizationManager localization(dir.GetPath().string() + "/");

		CHECK(localization.GetLanguageDisplayName(Language::German) == "Deutsch");
		CHECK(localization.GetLanguagePickerPrompt(Language::German) == "Wahle deine Sprache");
	}
}
