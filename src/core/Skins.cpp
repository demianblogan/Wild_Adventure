#include "Skins.h"

const std::array<Skin, 4>& AllSkins()
{
	static const std::array<Skin, 4> skins =
	{{
		{ "ninja_frog",  "Ninja Frog",  0 },
		{ "mask_dude",   "Mask Dude",   3 },
		{ "pink_man",    "Pink Man",    6 },
		{ "virtual_guy", "Virtual Guy", 7 },
	}};

	return skins;
}
