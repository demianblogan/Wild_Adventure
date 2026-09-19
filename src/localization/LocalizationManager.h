#pragma once

#include "localization/Language.h"

#include <string>
#include <unordered_map>

// Loads a flat "section.key = value" catalog for the active language from
// <catalogDirectory>/<code>.txt, with English always merged in first as a
// silent fallback for any key missing from the active language's file (so a
// partially translated language never shows a blank string).
class LocalizationManager
{
public:
	explicit LocalizationManager(std::string catalogDirectory = "assets/data/localization/");

	void SetLanguage(Language newLanguage);
	Language GetLanguage() const { return language; }

	// Returns the key itself if it is missing from both the active language
	// and the English fallback, so a gap is visible in-game instead of blank.
	std::string GetText(const std::string& key) const;

	// Resolves key like GetText, then replaces the first "{token}" in it with value.
	std::string FormatText(const std::string& key, const std::string& token, const std::string& value) const;

	// These read their own small standalone catalogs (language code -> text)
	// instead of the merged one above, since both must be readable for every
	// language at once (to list all five), including before any language has
	// been chosen.
	std::string GetLanguageDisplayName(Language forLanguage) const;
	std::string GetLanguagePickerPrompt(Language forLanguage) const;

	// Increases every time SetLanguage changes the active catalog, so a
	// screen holding JSON-driven UI can tell it needs to reload its panel to
	// pick up new text instead of polling GetLanguage() every frame.
	int Revision() const { return revision; }

private:
	std::string catalogDirectory;
	Language language = Language::English;
	std::unordered_map<std::string, std::string> catalog;
	int revision = 0;
};
