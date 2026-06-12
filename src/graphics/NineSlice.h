#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

namespace sf
{
	class RenderTarget;
	class Texture;
}

// Draws the texture as a 9-slice panel: the corners keep their pixel size,
// the edges and the center stretch to fill the requested size. Borders are
// in source-texture pixels. Shared by UI::Image and the hand-drawn menu
// screens (character select), so panels look identical everywhere.
void DrawNineSlice(sf::RenderTarget& target, const sf::Texture& texture,
	sf::Vector2f position, sf::Vector2f size,
	float borderLeft, float borderTop, float borderRight, float borderBottom,
	sf::Color color = sf::Color::White);
