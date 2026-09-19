#include "SafeFileWrite.h"

#include <fstream>
#include <string>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace
{
#ifdef _WIN32
	// Skips merging the temp file's ACLs/attributes into the target: a harmless
	// metadata mismatch between the two files must never turn a successful
	// save into a spurious failure.
	constexpr DWORD ReplaceFlags = REPLACEFILE_IGNORE_MERGE_ERRORS;
#endif

	std::filesystem::path TempPathFor(const std::filesystem::path& targetPath)
	{
		std::filesystem::path tempPath = targetPath;
		tempPath += ".tmp";
		return tempPath;
	}

	bool WriteWholeFile(const std::filesystem::path& path, std::string_view contents)
	{
		std::ofstream file(path, std::ios::binary | std::ios::trunc);
		if (!file.is_open())
			return false;

		file.write(contents.data(), static_cast<std::streamsize>(contents.size()));

		return file.good();
	}
}

namespace SafeFileWrite
{
	bool WriteFileAtomically(const std::filesystem::path& targetPath, std::string_view contents)
	{
		const std::filesystem::path tempPath = TempPathFor(targetPath);

		if (!WriteWholeFile(tempPath, contents))
		{
			std::error_code ignored;
			std::filesystem::remove(tempPath, ignored);
			return false;
		}

		std::error_code error;
		const bool targetExists = std::filesystem::exists(targetPath, error);

		if (!targetExists)
		{
			// Nothing to replace: creating targetPath is already one atomic
			// filesystem operation, on Windows and POSIX alike.
			std::filesystem::rename(tempPath, targetPath, error);
			return !error;
		}

#ifdef _WIN32
		// A single atomic kernel call: targetPath either still holds its old
		// content or fully holds the new one, even if the process dies mid-call.
		// Unlike a rename-old-away-then-rename-new-in two-step swap, there is no
		// window where targetPath is briefly missing.
		const bool replaced = ::ReplaceFileW(
			targetPath.c_str(), tempPath.c_str(), nullptr, ReplaceFlags, nullptr, nullptr) != 0;

		if (!replaced)
		{
			std::error_code ignored;
			std::filesystem::remove(tempPath, ignored);
		}

		return replaced;
#else
		// POSIX rename() atomically replaces an existing destination on the
		// same filesystem, so this is equally crash-safe without ReplaceFileW.
		std::filesystem::rename(tempPath, targetPath, error);
		return !error;
#endif
	}

	bool PreserveCorruptFile(const std::filesystem::path& path)
	{
		std::error_code error;
		std::filesystem::path corruptPath(path);
		corruptPath += ".corrupt";

		unsigned int duplicateIndex = 1;

		while (std::filesystem::exists(corruptPath, error) && !error)
		{
			corruptPath = path;
			corruptPath += ".corrupt." + std::to_string(duplicateIndex++);
		}

		std::filesystem::rename(path, corruptPath, error);

		return !error;
	}
}
