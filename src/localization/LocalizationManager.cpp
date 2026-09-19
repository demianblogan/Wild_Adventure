#include "LocalizationManager.h"

#include <fstream>
#include <utility>

namespace
{
	std::string Trim(const std::string& text)
	{
		const std::size_t begin = text.find_first_not_of(" \t\r\n");
		if (begin == std::string::npos)
			return "";

		const std::size_t end = text.find_last_not_of(" \t\r\n");
		return text.substr(begin, end - begin + 1);
	}

	// A catalog line is always a single physical line, so a multi-line value
	// (e.g. a confirmation dialog's body) spells its line breaks as the
	// two-character escape "\n" instead of an actual newline.
	std::string UnescapeNewlines(const std::string& text)
	{
		std::string result;
		result.reserve(text.size());

		for (std::size_t i = 0; i < text.size(); i++)
		{
			if (text[i] == '\\' && i + 1 < text.size() && text[i + 1] == 'n')
			{
				result += '\n';
				i++;
			}
			else
			{
				result += text[i];
			}
		}

		return result;
	}

	// Parses "key = value" lines (blank lines and lines starting with '#' are
	// skipped) into target; leaves target untouched if the file cannot be
	// opened, since a missing catalog must never crash the game.
	void LoadKeyValueFile(const std::string& path, std::unordered_map<std::string, std::string>& target)
	{
		std::ifstream file(path);
		if (!file.is_open())
			return;

		std::string line;
		while (std::getline(file, line))
		{
			if (line.empty() || line.front() == '#')
				continue;

			const std::size_t separator = line.find('=');
			if (separator == std::string::npos)
				continue;

			const std::string key = Trim(line.substr(0, separator));
			const std::string value = UnescapeNewlines(Trim(line.substr(separator + 1)));
			if (!key.empty())
				target[key] = value;
		}
	}
}

LocalizationManager::LocalizationManager(std::string catalogDirectory)
	: catalogDirectory(std::move(catalogDirectory))
{
}

void LocalizationManager::SetLanguage(Language newLanguage)
{
	language = newLanguage;

	catalog.clear();
	LoadKeyValueFile(catalogDirectory + LanguageCode(Language::English) + ".txt", catalog);
	if (newLanguage != Language::English)
		LoadKeyValueFile(catalogDirectory + LanguageCode(newLanguage) + ".txt", catalog);

	revision++;
}

std::string LocalizationManager::GetText(const std::string& key) const
{
	const auto it = catalog.find(key);
	return (it != catalog.end()) ? it->second : key;
}

std::string LocalizationManager::FormatText(const std::string& key, const std::string& token, const std::string& value) const
{
	std::string text = GetText(key);
	const std::string placeholder = "{" + token + "}";

	const std::size_t pos = text.find(placeholder);
	if (pos != std::string::npos)
		text.replace(pos, placeholder.length(), value);

	return text;
}

std::string LocalizationManager::GetLanguageDisplayName(Language forLanguage) const
{
	std::unordered_map<std::string, std::string> names;
	LoadKeyValueFile(catalogDirectory + "language_names.txt", names);

	const auto it = names.find(LanguageCode(forLanguage));
	return (it != names.end()) ? it->second : LanguageCode(forLanguage);
}

std::string LocalizationManager::GetLanguagePickerPrompt(Language forLanguage) const
{
	std::unordered_map<std::string, std::string> prompts;
	LoadKeyValueFile(catalogDirectory + "language_picker_prompt.txt", prompts);

	const auto it = prompts.find(LanguageCode(forLanguage));
	return (it != prompts.end()) ? it->second : "Choose your language";
}
