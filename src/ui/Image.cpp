#include "Image.h"

#include "core/Resources.h"

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
		const sf::Texture& texture = resources.textures.Get(textureName);
		const sf::Vector2u textureSize = texture.getSize();

		const float textureWidth = static_cast<float>(textureSize.x);
		const float textureHeight = static_cast<float>(textureSize.y);

		// Source-side column/row widths (texture pixels).
		const float sourceLeft = borderLeft;
		const float sourceRight = borderRight;
		const float sourceTop = borderTop;
		const float sourceBottom = borderBottom;
		const float sourceCenterX = std::max(0.0f, textureWidth - sourceLeft - sourceRight);
		const float sourceCenterY = std::max(0.0f, textureHeight - sourceTop - sourceBottom);

		// Destination-side column/row widths (screen pixels). Corners keep their size;
		// the center absorbs whatever is left after the fixed corners.
		const float destLeft = sourceLeft;
		const float destRight = sourceRight;
		const float destTop = sourceTop;
		const float destBottom = sourceBottom;
		const float destCenterX = std::max(0.0f, size.x - destLeft - destRight);
		const float destCenterY = std::max(0.0f, size.y - destTop - destBottom);

		// Column x-offsets and widths: [left, center, right].
		const float sourceX[3] = { 0.0f, sourceLeft, textureWidth - sourceRight };
		const float sourceW[3] = { sourceLeft, sourceCenterX, sourceRight };
		const float destXOff[3] = { 0.0f, destLeft, size.x - destRight };
		const float destW[3] = { destLeft, destCenterX, destRight };

		const float sourceY[3] = { 0.0f, sourceTop, textureHeight - sourceBottom };
		const float sourceH[3] = { sourceTop, sourceCenterY, sourceBottom };
		const float destYOff[3] = { 0.0f, destTop, size.y - destBottom };
		const float destH[3] = { destTop, destCenterY, destBottom };

		sf::Sprite sprite(texture);
		sprite.setColor(color);

		for (int column = 0; column < 3; column++)
		{
			if (sourceW[column] <= 0.0f || destW[column] <= 0.0f)
				continue;

			for (int row = 0; row < 3; row++)
			{
				if (sourceH[row] <= 0.0f || destH[row] <= 0.0f)
					continue;

				sprite.setTextureRect(sf::IntRect(
					{ static_cast<int>(sourceX[column]), static_cast<int>(sourceY[row]) },
					{ static_cast<int>(sourceW[column]), static_cast<int>(sourceH[row]) }));

				sprite.setPosition({ absolutePosition.x + destXOff[column], absolutePosition.y + destYOff[row] });
				sprite.setScale({ destW[column] / sourceW[column], destH[row] / sourceH[row] });

				target.draw(sprite);
			}
		}
	}
}