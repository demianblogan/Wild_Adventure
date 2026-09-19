#include "doctest/doctest.h"

#include "core/Campaign.h"
#include "core/SafeFileWrite.h"

#include "TempDirectory.h"

TEST_SUITE("Campaign")
{
	TEST_CASE("A fresh campaign has no progress")
	{
		Campaign campaign;

		CHECK_FALSE(campaign.HasProgress());
		CHECK(campaign.GetHighestCompletedLevel() == 0);
		CHECK(campaign.CountThreeStarLevels() == 0);
		CHECK_FALSE(campaign.IsLevelCompleted(1));
		CHECK(campaign.GetStars(1) == -1);
	}

	TEST_CASE("RecordCompletion keeps the best star count, never a worse one")
	{
		TempDirectory dir;
		Campaign campaign;
		campaign.Load((dir.GetPath() / "save.json").string());

		campaign.RecordCompletion(1, 2);
		CHECK(campaign.GetStars(1) == 2);

		campaign.RecordCompletion(1, 1); // a worse replay must not overwrite the best
		CHECK(campaign.GetStars(1) == 2);

		campaign.RecordCompletion(1, 3);
		CHECK(campaign.GetStars(1) == 3);
	}

	TEST_CASE("GetHighestCompletedLevel and CountThreeStarLevels reflect recorded completions")
	{
		TempDirectory dir;
		Campaign campaign;
		campaign.Load((dir.GetPath() / "save.json").string());

		campaign.RecordCompletion(1, 3);
		campaign.RecordCompletion(2, 3);
		campaign.RecordCompletion(3, 1);

		CHECK(campaign.GetHighestCompletedLevel() == 3);
		CHECK(campaign.CountThreeStarLevels() == 2);
		CHECK(campaign.IsLevelCompleted(3));
		CHECK_FALSE(campaign.IsLevelCompleted(4));
	}

	TEST_CASE("Save and Load round-trip stars, skin and victory flag")
	{
		const TempDirectory dir;
		const std::string savePath = (dir.GetPath() / "save.json").string();

		{
			Campaign campaign;
			campaign.Load(savePath);
			campaign.RecordCompletion(1, 2);
			campaign.RecordCompletion(3, 3);
			campaign.SetSelectedSkin("robot");
			campaign.MarkVictoryShown();
		}

		Campaign reloaded;
		reloaded.Load(savePath);

		CHECK(reloaded.GetStars(1) == 2);
		CHECK(reloaded.GetStars(3) == 3);
		CHECK(reloaded.GetSelectedSkin() == "robot");
		CHECK(reloaded.WasVictoryShown());
	}

	TEST_CASE("Loading a corrupt save falls back to a fresh campaign and preserves the file")
	{
		const TempDirectory dir;
		const std::filesystem::path savePath = dir.GetPath() / "save.json";

		REQUIRE(SafeFileWrite::WriteFileAtomically(savePath, "{ not valid json"));

		Campaign campaign;
		campaign.Load(savePath.string());

		CHECK_FALSE(campaign.HasProgress());
		CHECK_FALSE(std::filesystem::exists(savePath));
		CHECK(std::filesystem::exists(dir.GetPath() / "save.json.corrupt"));
	}

	TEST_CASE("Reset clears progress and deletes the save file")
	{
		const TempDirectory dir;
		const std::string savePath = (dir.GetPath() / "save.json").string();

		Campaign campaign;
		campaign.Load(savePath);
		campaign.RecordCompletion(1, 2);
		REQUIRE(std::filesystem::exists(savePath));

		campaign.Reset();

		CHECK_FALSE(campaign.HasProgress());
		CHECK_FALSE(std::filesystem::exists(savePath));
	}
}
