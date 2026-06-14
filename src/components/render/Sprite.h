#pragma once

#include <SFML/Graphics/Color.hpp>

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

		bool glow = false;                        // drawn into the bloom layer as well
		sf::Color glowColor = sf::Color::White;   // aura color (flat silhouette)
	};
}