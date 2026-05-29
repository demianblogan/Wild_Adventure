#pragma once

#include "ui/Element.h"

#include <SFML/Graphics/Color.hpp>

#include <string>

struct Resources;

namespace UI
{
	class Image : public Element
	{
	public:
		Image(Resources& resources);

		void SetTexture(const std::string& textureName);
		void SetColor(sf::Color color);
		void SetAlpha(float alpha);

	protected:
		void DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const override;

	private:
		Resources& resources;
		std::string textureName;
		sf::Color color = sf::Color::White;
	};
}