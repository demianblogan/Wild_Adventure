#include "Campaign.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>

Campaign::Campaign()
{
	bestStars.fill(-1);
}

void Campaign::Load(const std::string& path)
{
	savePath = path;
	bestStars.fill(-1);
	victoryShown = false;
	selectedSkin = "ninja_frog";

	std::ifstream file(path);

	// No save file yet: a fresh campaign.
	if (!file.is_open())
		return;

	const nlohmann::json data = nlohmann::json::parse(file);

	victoryShown = data.value("victoryShown", false);
	selectedSkin = data.value("selectedSkin", std::string("ninja_frog"));

	if (!data.contains("levels"))
		return;

	for (const auto& [key, level] : data.at("levels").items())
	{
		const int number = std::stoi(key);

		if (number >= 1 && number <= LEVEL_COUNT)
			bestStars[number - 1] = level.value("stars", 0);
	}
}

void Campaign::Save() const
{
	if (savePath.empty())
		return;

	nlohmann::json data;
	data["victoryShown"] = victoryShown;
	data["selectedSkin"] = selectedSkin;
	data["levels"] = nlohmann::json::object();

	for (int i = 0; i < LEVEL_COUNT; i++)
	{
		if (bestStars[i] >= 0)
			data["levels"][std::to_string(i + 1)]["stars"] = bestStars[i];
	}

	std::ofstream file(savePath);

	if (!file.is_open())
		throw std::runtime_error("Campaign: cannot write '" + savePath + "'");

	file << data.dump(1, '\t');
}

void Campaign::RecordCompletion(int levelNumber, int stars)
{
	if (levelNumber < 1 || levelNumber > LEVEL_COUNT)
		return;

	int& best = bestStars[levelNumber - 1];

	if (stars > best) // -1 means not completed, so any completion improves it
		best = stars;

	Save();
}

void Campaign::Reset()
{
	bestStars.fill(-1);
	victoryShown = false;
	selectedSkin = "ninja_frog";

	if (savePath.empty())
		return;

	std::error_code error; // ignore: a missing file is already the desired result
	std::filesystem::remove(savePath, error);
}

bool Campaign::HasProgress() const
{
	return GetHighestCompletedLevel() > 0;
}

bool Campaign::IsLevelCompleted(int levelNumber) const
{
	return levelNumber >= 1 && levelNumber <= LEVEL_COUNT && bestStars[levelNumber - 1] >= 0;
}

int Campaign::GetStars(int levelNumber) const
{
	if (levelNumber < 1 || levelNumber > LEVEL_COUNT)
		return -1;

	return bestStars[levelNumber - 1];
}

int Campaign::GetHighestCompletedLevel() const
{
	int highest = 0;

	for (int i = 0; i < LEVEL_COUNT; i++)
	{
		if (bestStars[i] >= 0)
			highest = i + 1;
	}

	return highest;
}

int Campaign::CountThreeStarLevels() const
{
	int count = 0;

	for (int i = 0; i < LEVEL_COUNT; i++)
	{
		if (bestStars[i] >= 3)
			count++;
	}

	return count;
}

const std::string& Campaign::GetSelectedSkin() const
{
	return selectedSkin;
}

void Campaign::SetSelectedSkin(const std::string& skinId)
{
	if (selectedSkin == skinId)
		return;

	selectedSkin = skinId;
	Save();
}

bool Campaign::WasVictoryShown() const
{
	return victoryShown;
}

void Campaign::MarkVictoryShown()
{
	if (victoryShown)
		return;

	victoryShown = true;
	Save();
}

std::string Campaign::LevelPath(int levelNumber)
{
	return "data/levels/level_" + std::to_string(levelNumber) + ".tmj";
}

bool Campaign::LevelExists(int levelNumber)
{
	if (levelNumber < 1 || levelNumber > LEVEL_COUNT)
		return false;

	return std::ifstream(LevelPath(levelNumber)).is_open();
}

bool Campaign::IsLastLevel(int levelNumber)
{
	return LevelExists(levelNumber) && !LevelExists(levelNumber + 1);
}