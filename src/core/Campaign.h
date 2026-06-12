#pragma once

#include <array>
#include <string>

// Campaign progress: which levels are completed and the best star count earned
// on each. Stars: 1 — no deaths, 2 — all fruits collected, 3 — all enemies killed.
class Campaign
{
public:
	static constexpr int LEVEL_COUNT = 9;

	Campaign();

	void Load(const std::string& path);

	// Records a completion, keeps the best star count and saves to disk.
	void RecordCompletion(int levelNumber, int stars);

	// Wipes all progress and deletes the save file from disk.
	void Reset();

	bool HasProgress() const;

	bool IsLevelCompleted(int levelNumber) const;
	int GetStars(int levelNumber) const;          // -1 when not completed
	int GetHighestCompletedLevel() const;         // 0 when nothing is completed
	int CountThreeStarLevels() const;             // drives character skin unlocks

	// The character skin chosen on the Select Character screen, persisted
	// with the campaign progress.
	const std::string& GetSelectedSkin() const;
	void SetSelectedSkin(const std::string& skinId);

	// The campaign victory screen is shown only once per campaign.
	bool WasVictoryShown() const;
	void MarkVictoryShown();

	static std::string LevelPath(int levelNumber);
	static bool LevelExists(int levelNumber);
	static bool IsLastLevel(int levelNumber);     // exists and has no next level

private:
	void Save() const;

	std::string savePath;
	std::array<int, LEVEL_COUNT> bestStars; // -1 = not completed, 0..3 = best stars
	bool victoryShown = false;
	std::string selectedSkin = "ninja_frog";
};
