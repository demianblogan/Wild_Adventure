#include "LightOverlay.h"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{
	float SmoothStep(float edge0, float edge1, float x)
	{
		const float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		return t * t * (3.0f - 2.0f * t);
	}
}

LightOverlay::LightOverlay()
{
	// White disc whose alpha rises smoothly from 0 (lit center) to full
	// (fully opaque) toward the edge; the square's corners reach full alpha,
	// so the sprite blends seamlessly into the filler rectangles around it.
	// Stored as white (not the tint color) so Draw's `tint` parameter can
	// recolor it freely -- multiplying by white leaves a color unchanged.
	sf::Image image(sf::Vector2u{ TextureSize, TextureSize }, sf::Color::Transparent);

	const float half = TextureSize / 2.0f;

	for (unsigned int y = 0; y < TextureSize; y++)
	{
		for (unsigned int x = 0; x < TextureSize; x++)
		{
			const float dx = (static_cast<float>(x) + 0.5f - half) / half;
			const float dy = (static_cast<float>(y) + 0.5f - half) / half;
			const float distance = std::sqrt(dx * dx + dy * dy);

			const float shade = SmoothStep(InnerFraction, 1.0f, distance);
			const auto alpha = static_cast<std::uint8_t>(shade * 255.0f);

			image.setPixel(sf::Vector2u{ x, y }, sf::Color(255, 255, 255, alpha));
		}
	}

	if (!gradientTexture.loadFromImage(image))
		return; // Draw still works: the rectangles tint everything but the circle

	gradientTexture.setSmooth(true);
}

void LightOverlay::Draw(sf::RenderTarget& target, sf::Vector2f center, float radius, float intensity, sf::Color tint)
{
	if (radius <= 0.0f || intensity <= 0.0f)
		return;

	const auto alpha = static_cast<std::uint8_t>(std::clamp(intensity, 0.0f, 1.0f) * 255.0f);
	const sf::Color scaledTint(tint.r, tint.g, tint.b, alpha);

	// The gradient sprite over the circle's bounding box. Its own alpha is
	// scaled by the overall intensity, and its white RGB takes on `tint`,
	// through the sprite color.
	sf::Sprite sprite(gradientTexture);
	sprite.setOrigin({ TextureSize / 2.0f, TextureSize / 2.0f });
	const float scale = (radius * 2.0f) / static_cast<float>(TextureSize);
	sprite.setScale({ scale, scale });
	sprite.setPosition(center);
	sprite.setColor(scaledTint);
	target.draw(sprite);

	// Four solid bands covering the view outside the circle's bounding box.
	const sf::View& view = target.getView();
	const float viewLeft   = view.getCenter().x - view.getSize().x / 2.0f;
	const float viewTop    = view.getCenter().y - view.getSize().y / 2.0f;
	const float viewRight  = viewLeft + view.getSize().x;
	const float viewBottom = viewTop + view.getSize().y;

	const float boxLeft   = center.x - radius;
	const float boxTop    = center.y - radius;
	const float boxRight  = center.x + radius;
	const float boxBottom = center.y + radius;

	const auto drawBand = [&](float left, float top, float right, float bottom)
	{
		if (right <= left || bottom <= top)
			return;

		sf::RectangleShape band({ right - left, bottom - top });
		band.setPosition({ left, top });
		band.setFillColor(scaledTint);
		target.draw(band);
	};

	drawBand(viewLeft, viewTop, viewRight, boxTop);                                    // above
	drawBand(viewLeft, boxBottom, viewRight, viewBottom);                              // below
	drawBand(viewLeft, std::max(viewTop, boxTop), boxLeft, std::min(viewBottom, boxBottom));   // left
	drawBand(boxRight, std::max(viewTop, boxTop), viewRight, std::min(viewBottom, boxBottom)); // right
}
