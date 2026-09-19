#pragma once

#include <string>

enum class Language
{
	English,
	German,
	Spanish,
	Russian,
	Ukrainian
};

// Used for catalog file names (assets/data/localization/<code>.txt) and for
// persisting the setting; LanguageFromCode is its inverse (falls back to
// English for an unrecognized code, e.g. a hand-edited settings file).
std::string LanguageCode(Language language);
Language LanguageFromCode(const std::string& code);
