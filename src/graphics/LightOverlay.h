#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

namespace sf
{
	class RenderTarget;
}

// Tints everything outside a soft-edged circle around a world (or screen)
// position: a radial-gradient sprite covers the circle, four solid rectangles
// cover the rest of the view. The gradient texture (a plain alpha falloff,
// stored as white so `tint` controls the actual color) is generated in code,
// so no asset or shader is needed.
//
// Used both for the player's "lamp" on cave levels (tint = black, a circle
// centered on the player) and for a full-screen critical-health vignette
// (tint = red, centered on the screen with a radius past its corners so only
// the edges show).
class LightOverlay
{
public:
	LightOverlay();

	// intensity is the maximum alpha outside the circle, 0..1 (1 = fully opaque
	// tint). Call with the camera view active: the filler rectangles are sized
	// to it.
	void Draw(sf::RenderTarget& target, sf::Vector2f center, float radius, float intensity,
		sf::Color tint = sf::Color::Black);

private:
	sf::Texture gradientTexture;

	static constexpr unsigned int TextureSize = 256;
	static constexpr float InnerFraction = 0.45f; // fully lit portion of the radius
};
