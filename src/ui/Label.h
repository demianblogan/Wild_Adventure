#pragma once

#include "ui/Element.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>

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
		void SetColor(sf::Color color) override;
		sf::Color GetColor() const { return color; }
		void SetAlpha(float alpha);
		void SetOutlineColor(sf::Color color);
		void SetOutlineThickness(float thickness);

	protected:
		void DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const override;

	private:
		void RecalculateSize();

		// Rebuilding an sf::Text regenerates its glyph geometry, which is only
		// necessary when the string/font/size actually change - not on every
		// draw. Keeping one cached instance and just re-coloring/repositioning
		// it each frame is what keeps text-heavy screens (many Labels redrawn
		// every frame) from costing far more than they need to.
		mutable sf::Text drawableText;

		sf::Color color = sf::Color::White;
		sf::Color outlineColor = sf::Color::Transparent;
		float outlineThickness = 0.0f;
	};
}