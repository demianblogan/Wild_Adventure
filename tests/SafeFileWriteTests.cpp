#include "doctest/doctest.h"

#include "core/SafeFileWrite.h"

#include "TempDirectory.h"

#include <fstream>
#include <sstream>

namespace
{
	std::string ReadWholeFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::binary);
		std::ostringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}
}

TEST_SUITE("SafeFileWrite")
{
	TEST_CASE("WriteFileAtomically creates a file that does not exist yet")
	{
		TempDirectory dir;
		const std::filesystem::path target = dir.GetPath() / "save.json";

		CHECK(SafeFileWrite::WriteFileAtomically(target, "hello"));

		REQUIRE(std::filesystem::exists(target));
		CHECK(ReadWholeFile(target) == "hello");

		// No leftover temp file after a successful write.
		CHECK_FALSE(std::filesystem::exists(dir.GetPath() / "save.json.tmp"));
	}

	TEST_CASE("WriteFileAtomically replaces an existing file's content")
	{
		TempDirectory dir;
		const std::filesystem::path target = dir.GetPath() / "save.json";

		REQUIRE(SafeFileWrite::WriteFileAtomically(target, "first"));
		REQUIRE(SafeFileWrite::WriteFileAtomically(target, "second, and longer"));

		CHECK(ReadWholeFile(target) == "second, and longer");
		CHECK_FALSE(std::filesystem::exists(dir.GetPath() / "save.json.tmp"));
	}

	TEST_CASE("WriteFileAtomically never leaves the target missing between two writes")
	{
		TempDirectory dir;
		const std::filesystem::path target = dir.GetPath() / "save.json";

		REQUIRE(SafeFileWrite::WriteFileAtomically(target, "original"));
		REQUIRE(SafeFileWrite::WriteFileAtomically(target, "updated"));

		// A failed second write must not have deleted the original content;
		// here it succeeded, so the file must hold exactly one full version.
		REQUIRE(std::filesystem::exists(target));
		CHECK(ReadWholeFile(target) == "updated");
	}

	TEST_CASE("PreserveCorruptFile renames the file aside")
	{
		TempDirectory dir;
		const std::filesystem::path original = dir.GetPath() / "save.json";
		const std::filesystem::path corrupt = dir.GetPath() / "save.json.corrupt";

		REQUIRE(SafeFileWrite::WriteFileAtomically(original, "not valid json"));

		CHECK(SafeFileWrite::PreserveCorruptFile(original));

		CHECK_FALSE(std::filesystem::exists(original));
		REQUIRE(std::filesystem::exists(corrupt));
		CHECK(ReadWholeFile(corrupt) == "not valid json");
	}

	TEST_CASE("PreserveCorruptFile numbers duplicates instead of overwriting them")
	{
		TempDirectory dir;
		const std::filesystem::path original = dir.GetPath() / "save.json";

		REQUIRE(SafeFileWrite::WriteFileAtomically(original, "attempt 1"));
		REQUIRE(SafeFileWrite::PreserveCorruptFile(original));

		REQUIRE(SafeFileWrite::WriteFileAtomically(original, "attempt 2"));
		REQUIRE(SafeFileWrite::PreserveCorruptFile(original));

		CHECK(ReadWholeFile(dir.GetPath() / "save.json.corrupt") == "attempt 1");
		CHECK(ReadWholeFile(dir.GetPath() / "save.json.corrupt.1") == "attempt 2");
	}
}
