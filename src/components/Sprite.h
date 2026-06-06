#pragma once

#include <string>

namespace ECS
{
	struct Sprite
	{
		std::string textureName;
		float offsetX = 0.0f;
		float offsetY = 0.0f; // visual nudge; positive = down
	};
}