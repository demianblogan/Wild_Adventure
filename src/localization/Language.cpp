#include "Language.h"

std::string LanguageCode(Language language)
{
	switch (language)
	{
	case Language::English:   return "en";
	case Language::German:    return "de";
	case Language::Spanish:   return "es";
	case Language::Russian:   return "ru";
	case Language::Ukrainian: return "uk";
	}
	return "en";
}

Language LanguageFromCode(const std::string& code)
{
	if (code == "de") return Language::German;
	if (code == "es") return Language::Spanish;
	if (code == "ru") return Language::Russian;
	if (code == "uk") return Language::Ukrainian;
	return Language::English;
}
