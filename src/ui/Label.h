#pragma once

#include "ui/Element.h"

#include <SFML/Graphics/Color.hpp>

#include <string>

struct Resources;

namespace UI
{
	class Label : public Element
	{
	public:
		Label(Resources& resources, const std::string& fontName);

		void SetText(const std::string& text);
		void SetCharacterSize(unsigned int characterSize);
		void SetColor(sf::Color color);
		void SetAlpha(float alpha);

	protected:
		void DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const override;

	private:
		void RecalculateSize();

		Resources& resources;
		std::string fontName;
		std::string text;
		unsigned int characterSize = 16;
		sf::Color color = sf::Color::White;
	};
}