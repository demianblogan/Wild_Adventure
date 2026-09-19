#pragma once

#include "core/Random.h"

#include <filesystem>
#include <string>
#include <system_error>

// RAII scratch directory under the OS temp folder, unique per instance and
// removed when it goes out of scope. Keeps file-writing tests from touching
// the repository's real data/ files or leaking files between test runs.
class TempDirectory
{
public:
	TempDirectory()
		: path(std::filesystem::temp_directory_path() / UniqueDirectoryName())
	{
		std::filesystem::create_directories(path);
	}

	~TempDirectory()
	{
		std::error_code ignored;
		std::filesystem::remove_all(path, ignored);
	}

	TempDirectory(const TempDirectory&) = delete;
	TempDirectory& operator=(const TempDirectory&) = delete;

	const std::filesystem::path& GetPath() const { return path; }

private:
	static std::string UniqueDirectoryName()
	{
		static constexpr int MAX_SUFFIX = 2'000'000'000;
		return "WildAdventureTests_" + std::to_string(Random::Int(0, MAX_SUFFIX));
	}

	std::filesystem::path path;
};
