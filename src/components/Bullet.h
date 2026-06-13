#pragma once

#include <string>

namespace ECS
{
	struct Bullet
	{
		int         dirX          = 1;  // horizontal travel: +1 right, -1 left, 0 none
		int         dirY          = 0;  // vertical travel:   +1 down,  -1 up,   0 none
		std::string piecesTexture = "trunk_bullet_pieces"; // debris sheet emitted on impact
	};
}
