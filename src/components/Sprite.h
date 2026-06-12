#pragma once

#include <string>

namespace ECS
{
	struct Sprite
	{
		std::string textureName;
		float offsetX = 0.0f;
		float offsetY = 0.0f; // visual nudge; positive = down

		// Visual-only squash & stretch, applied around the sprite's bottom-center
		// origin so feet stay planted. Purely cosmetic: colliders are unaffected.
		float scaleX = 1.0f;
		float scaleY = 1.0f;
	};
}