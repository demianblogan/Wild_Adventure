#include "doctest/doctest.h"

#include "core/AppDataPath.h"

TEST_SUITE("AppDataPath")
{
	TEST_CASE("Resolve places the file under Alone Bull Company/Wild Adventure")
	{
		const std::filesystem::path path = AppDataPath::Resolve("settings.json");

		CHECK(path.filename() == "settings.json");
		CHECK(path.parent_path().filename() == "Wild Adventure");
		CHECK(path.parent_path().parent_path().filename() == "Alone Bull Company");
	}

	TEST_CASE("Resolve gives different file names different full paths")
	{
		CHECK(AppDataPath::Resolve("settings.json") != AppDataPath::Resolve("save.json"));
	}
}
