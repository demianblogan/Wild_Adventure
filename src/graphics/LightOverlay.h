#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

namespace sf
{
	class RenderTarget;
}

// Darkens everything outside a soft-edged circle of light around a world
// position (the player's "lamp" on cave levels): a radial-gradient sprite
// covers the circle, four solid rectangles cover the rest of the view.
// The gradient texture is generated in code, so no asset or shader is needed.
class LightOverlay
{
public:
	LightOverlay();

	// darkness is the maximum shade outside the circle, 0..1 (1 = pure black).
	// Call with the camera view active: the filler rectangles are sized to it.
	void Draw(sf::RenderTarget& target, sf::Vector2f lightCenter, float radius, float darkness);

private:
	sf::Texture gradientTexture;

	static constexpr unsigned int TEXTURE_SIZE = 256;
	static constexpr float INNER_FRACTION = 0.45f; // fully lit portion of the radius
};
