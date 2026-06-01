#include "Image.h"

#include "core/Resources.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace UI
{
	Image::Image(Resources& resources)
		: resources(resources)
	{}

	void Image::SetTexture(const std::string& textureName)
	{
		this->textureName = textureName;
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

	void Image::DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const
	{
		if (textureName.empty())
		{
			sf::RectangleShape rectangle(size);
			rectangle.setPosition(absolutePosition);
			rectangle.setFillColor(color);

			target.draw(rectangle);
		}
		else
		{
			const sf::Texture& texture = resources.textures.Get(textureName);

			sf::Sprite sprite(texture);
			sprite.setPosition(absolutePosition);

			const sf::Vector2u textureSize = texture.getSize();
			sprite.setScale({
				size.x / static_cast<float>(textureSize.x),
				size.y / static_cast<float>(textureSize.y) });

			target.draw(sprite);
		}
	}
}