#include "Campaign.h"

#include "core/SafeFileWrite.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

Campaign::Campaign()
{
	bestStars.fill(-1);
}

void Campaign::Load(const std::string& path)
{
	savePath = path;
	bestStars.fill(-1);
	wasVictoryShown = false;
	selectedSkin = "ninja_frog";

	std::ifstream file(path);

	// No save file yet: a fresh campaign.
	if (!file.is_open())
		return;

	try
	{
		const nlohmann::json data = nlohmann::json::parse(file);

		wasVictoryShown = data.value("victoryShown", false);
		selectedSkin = data.value("selectedSkin", std::string("ninja_frog"));

		if (!data.contains("levels"))
			return;

		for (const auto& [key, level] : data.at("levels").items())
		{
			const int number = std::stoi(key);

			if (number >= 1 && number <= LevelCount)
				bestStars[number - 1] = level.value("stars", 0);
		}
	}
	catch (const std::exception&)
	{
		// A hand-edited or crash-truncated save must never take the game down
		// with it: reset to a fresh campaign and keep the bad file around
		// (renamed aside) for inspection instead of silently overwriting it.
		bestStars.fill(-1);
		wasVictoryShown = false;
		selectedSkin = "ninja_frog";

		file.close();
		static_cast<void>(SafeFileWrite::PreserveCorruptFile(path));
	}
}

void Campaign::Save() const
{
	if (savePath.empty())
		return;

	nlohmann::json data;
	data["victoryShown"] = wasVictoryShown;
	data["selectedSkin"] = selectedSkin;
	data["levels"] = nlohmann::json::object();

	for (int i = 0; i < LevelCount; i++)
	{
		if (bestStars[i] >= 0)
			data["levels"][std::to_string(i + 1)]["stars"] = bestStars[i];
	}

	static_cast<void>(SafeFileWrite::WriteFileAtomically(savePath, data.dump(1, '\t')));
}

void Campaign::RecordCompletion(int levelNumber, int stars)
{
	if (levelNumber < 1 || levelNumber > LevelCount)
		return;

	int& best = bestStars[levelNumber - 1];

	if (stars > best) // -1 means not completed, so any completion improves it
		best = stars;

	Save();
}

void Campaign::Reset()
{
	bestStars.fill(-1);
	wasVictoryShown = false;
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
	return levelNumber >= 1 && levelNumber <= LevelCount && bestStars[levelNumber - 1] >= 0;
}

int Campaign::GetStars(int levelNumber) const
{
	if (levelNumber < 1 || levelNumber > LevelCount)
		return -1;

	return bestStars[levelNumber - 1];
}

int Campaign::GetHighestCompletedLevel() const
{
	int highest = 0;

	for (int i = 0; i < LevelCount; i++)
	{
		if (bestStars[i] >= 0)
			highest = i + 1;
	}

	return highest;
}

int Campaign::CountThreeStarLevels() const
{
	int count = 0;

	for (int i = 0; i < LevelCount; i++)
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
	return wasVictoryShown;
}

void Campaign::MarkVictoryShown()
{
	if (wasVictoryShown)
		return;

	wasVictoryShown = true;
	Save();
}

std::string Campaign::LevelPath(int levelNumber)
{
	return "data/levels/level_" + std::to_string(levelNumber) + ".tmj";
}

bool Campaign::LevelExists(int levelNumber)
{
	if (levelNumber < 1 || levelNumber > LevelCount)
		return false;

	return std::ifstream(LevelPath(levelNumber)).is_open();
}

bool Campaign::IsLastLevel(int levelNumber)
{
	return LevelExists(levelNumber) && !LevelExists(levelNumber + 1);
}