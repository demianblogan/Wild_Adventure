#include "Image.h"

#include "core/Resources.h"
#include "graphics/NineSlice.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>

namespace UI
{
	Image::Image(Resources& resources)
		: resources(resources)
	{}

	void Image::SetTexture(const std::string& textureName)
	{
		this->textureName = textureName;
	}

	void Image::SetTextureRect(sf::IntRect rect)
	{
		textureRect = rect;
	}

	void Image::SetColor(sf::Color color)
	{
		this->color = color;
	}

	void Image::SetAlpha(float alpha)
	{
		std::uint8_t byteAlpha = static_cast<std::uint8_t>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f);
		color.a = byteAlpha;
	}

	void Image::SetBorder(float left, float top, float right, float bottom)
	{
		borderLeft = left;
		borderTop = top;
		borderRight = right;
		borderBottom = bottom;
	}

	bool Image::HasBorder() const
	{
		return borderLeft > 0.0f || borderTop > 0.0f || borderRight > 0.0f || borderBottom > 0.0f;
	}

	void Image::DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const
	{
		if (textureName.empty())
		{
			sf::RectangleShape rectangle(size);
			rectangle.setPosition(absolutePosition);
			rectangle.setFillColor(color);
			target.draw(rectangle);
			return;
		}

		if (HasBorder())
			DrawNineSlice(target, absolutePosition);
		else
			DrawStretched(target, absolutePosition);
	}

	void Image::DrawStretched(sf::RenderTarget& target, sf::Vector2f absolutePosition) const
	{
		const sf::Texture& texture = resources.textures.Get(textureName);

		sf::Sprite sprite(texture);
		sprite.setPosition(absolutePosition);
		sprite.setColor(color);

		// A sub-rectangle (spritesheet glyph) takes priority over the full texture.
		const sf::Vector2f sourceSize = textureRect.has_value()
			? sf::Vector2f(textureRect->size)
			: sf::Vector2f(texture.getSize());

		if (textureRect.has_value())
			sprite.setTextureRect(textureRect.value());

		sprite.setScale({
			size.x / sourceSize.x,
			size.y / sourceSize.y });

		target.draw(sprite);
	}

	void Image::DrawNineSlice(sf::RenderTarget& target, sf::Vector2f absolutePosition) const
	{
		::DrawNineSlice(target, resources.textures.Get(textureName), absolutePosition, size,
			borderLeft, borderTop, borderRight, borderBottom, color);
	}
}