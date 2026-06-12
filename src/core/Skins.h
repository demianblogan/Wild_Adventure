#pragma once

#include <array>
#include <string>

// Playable character skins. Each skin's sheets live in
// assets/textures/main_characters/<id>/ with identical animations and sizes,
// registered in the texture manifests as <id>_idle, <id>_run and so on.
// The player prefab is authored with the ninja_frog_* names; GameState
// rewrites the prefix when another skin is selected.
struct Skin
{
	std::string id;          // texture-name prefix == folder name
	std::string displayName;
	int requiredThreeStars;  // levels completed with 3 stars needed to unlock
};

inline const std::array<Skin, 4>& AllSkins()
{
	static const std::array<Skin, 4> skins =
	{{
		{ "ninja_frog",  "Ninja Frog",  0 },
		{ "mask_dude",   "Mask Dude",   3 },
		{ "pink_man",    "Pink Man",    6 },
		{ "virtual_guy", "Virtual Guy", 9 },
	}};

	return skins;
}
