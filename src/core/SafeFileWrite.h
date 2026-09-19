#pragma once

#include <filesystem>
#include <string_view>

// Crash-safe file writing for save data. The new content goes to a temp file
// first; WriteFileAtomically then swaps it into targetPath's place with a
// single atomic OS call (ReplaceFileW on Windows, POSIX rename elsewhere),
// so a crash, power loss or a lock mid-write can never leave targetPath
// missing or half-written -- at worst the previous, still-valid file survives.
namespace SafeFileWrite
{
	// Writes `contents` to targetPath. Returns false, leaving targetPath
	// untouched, if either the temp write or the atomic swap fails.
	[[nodiscard]] bool WriteFileAtomically(const std::filesystem::path& targetPath, std::string_view contents);

	// Renames a file that failed to load/parse to <path>.corrupt (or
	// .corrupt.1, .corrupt.2, ... if that name is taken) instead of deleting
	// or silently overwriting it, so a corrupted save stays on disk for
	// inspection while the caller falls back to defaults.
	[[nodiscard]] bool PreserveCorruptFile(const std::filesystem::path& path);
}
