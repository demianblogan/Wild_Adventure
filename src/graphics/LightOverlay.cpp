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
	// Black disc whose alpha rises smoothly from 0 (lit center) to full
	// (darkness) toward the edge; the square's corners are fully dark, so the
	// sprite blends seamlessly into the filler rectangles around it.
	sf::Image image(sf::Vector2u{ TEXTURE_SIZE, TEXTURE_SIZE }, sf::Color::Transparent);

	const float half = TEXTURE_SIZE / 2.0f;

	for (unsigned int y = 0; y < TEXTURE_SIZE; y++)
	{
		for (unsigned int x = 0; x < TEXTURE_SIZE; x++)
		{
			const float dx = (static_cast<float>(x) + 0.5f - half) / half;
			const float dy = (static_cast<float>(y) + 0.5f - half) / half;
			const float distance = std::sqrt(dx * dx + dy * dy);

			const float shade = SmoothStep(INNER_FRACTION, 1.0f, distance);
			const auto alpha = static_cast<std::uint8_t>(shade * 255.0f);

			image.setPixel(sf::Vector2u{ x, y }, sf::Color(0, 0, 0, alpha));
		}
	}

	if (!gradientTexture.loadFromImage(image))
		return; // Draw still works: the rectangles darken everything but the circle

	gradientTexture.setSmooth(true);
}

void LightOverlay::Draw(sf::RenderTarget& target, sf::Vector2f lightCenter, float radius, float darkness)
{
	if (radius <= 0.0f || darkness <= 0.0f)
		return;

	const auto alpha = static_cast<std::uint8_t>(std::clamp(darkness, 0.0f, 1.0f) * 255.0f);

	// The gradient sprite over the circle's bounding box. Its own alpha is
	// scaled by the overall darkness through the sprite color.
	sf::Sprite sprite(gradientTexture);
	sprite.setOrigin({ TEXTURE_SIZE / 2.0f, TEXTURE_SIZE / 2.0f });
	const float scale = (radius * 2.0f) / static_cast<float>(TEXTURE_SIZE);
	sprite.setScale({ scale, scale });
	sprite.setPosition(lightCenter);
	sprite.setColor(sf::Color(255, 255, 255, alpha));
	target.draw(sprite);

	// Four solid bands covering the view outside the circle's bounding box.
	const sf::View& view = target.getView();
	const float viewLeft   = view.getCenter().x - view.getSize().x / 2.0f;
	const float viewTop    = view.getCenter().y - view.getSize().y / 2.0f;
	const float viewRight  = viewLeft + view.getSize().x;
	const float viewBottom = viewTop + view.getSize().y;

	const float boxLeft   = lightCenter.x - radius;
	const float boxTop    = lightCenter.y - radius;
	const float boxRight  = lightCenter.x + radius;
	const float boxBottom = lightCenter.y + radius;

	const sf::Color dark(0, 0, 0, alpha);

	const auto drawBand = [&](float left, float top, float right, float bottom)
	{
		if (right <= left || bottom <= top)
			return;

		sf::RectangleShape band({ right - left, bottom - top });
		band.setPosition({ left, top });
		band.setFillColor(dark);
		target.draw(band);
	};

	drawBand(viewLeft, viewTop, viewRight, boxTop);                                    // above
	drawBand(viewLeft, boxBottom, viewRight, viewBottom);                              // below
	drawBand(viewLeft, std::max(viewTop, boxTop), boxLeft, std::min(viewBottom, boxBottom));   // left
	drawBand(boxRight, std::max(viewTop, boxTop), viewRight, std::min(viewBottom, boxBottom)); // right
}
