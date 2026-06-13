#pragma once

#include <string>

namespace ECS
{
	struct Bullet
	{
		int         direction     = 1; // +1 right, -1 left
		std::string piecesTexture = "trunk_bullet_pieces"; // debris sheet emitted on impact
	};
}
