#pragma once

#include <filesystem>
#include <string_view>

// Where per-player save/config files live (settings, keyboard bindings,
// campaign progress) -- mirrors the convention already used by this
// developer's other projects (Tessera, ULA).
namespace AppDataPath
{
	// Full path to a per-player file named fileName. Prefers
	// %LOCALAPPDATA%\Alone Bull Company\Wild Adventure\<fileName> -- the
	// standard per-user, non-roaming location Windows recommends for this
	// kind of data. Falls back to a user_data\ folder next to the executable
	// if LOCALAPPDATA can't be read.
	[[nodiscard]] std::filesystem::path Resolve(std::string_view fileName);
}
