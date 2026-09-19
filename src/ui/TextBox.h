#pragma once

#include "ui/Element.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>

#include <optional>
#include <string>
#include <vector>

struct Resources;

namespace UI
{
	// A rectangular text container: give it a width and a single string (no
	// manual '\n' wrapping) and it word-wraps to fit, then lays out every
	// line itself using the font's own line-spacing metric so rows stay
	// evenly spaced regardless of what glyphs (ascenders/descenders) any one
	// line happens to contain - unlike Label, which anchors each line by its
	// own tight ink bounds and drifts line-to-line as a result.
	//
	// A single-line "name\tvalue" string (a literal tab) is a special case:
	// the two halves are drawn back to back in two different colors, with the
	// gap between them coming from the *measured* width of the name - not a
	// guessed pixel offset - so a "Name:" column of any length lines up
	// flush against its value with the same gap every time.
	class TextBox : public Element
	{
	public:
		enum class Alignment { Left, Center, Right };

		TextBox(Resources& resources, const std::string& fontName);

		void SetText(const std::string& text);
		void SetCharacterSize(unsigned int characterSize);
		void SetColor(sf::Color color) override;
		void SetOutlineColor(sf::Color color);
		void SetOutlineThickness(float thickness);
		void SetAlignment(Alignment alignment);

		// Second color/outline for the "value" half of a "name\tvalue" string.
		void SetSecondColor(sf::Color color);
		void SetSecondOutlineColor(sf::Color color);

	protected:
		void DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const override;

	private:
		void Rebuild();
		void ApplyColors();

		Resources& resources;
		std::string fontName;
		std::string text;
		unsigned int characterSize = 16;
		sf::Color color = sf::Color::White;
		sf::Color outlineColor = sf::Color::Transparent;
		sf::Color secondColor = sf::Color::White;
		sf::Color secondOutlineColor = sf::Color::Transparent;
		float outlineThickness = 0.0f;
		Alignment alignment = Alignment::Center;

		// Cached drawable text objects; rebuilt only when the wrapped content
		// actually changes, not on every draw (rebuilding an sf::Text redoes
		// its glyph layout, which is wasted work if nothing changed since the
		// last frame).
		mutable std::vector<sf::Text> lineTexts;

		// Set only in "name\tvalue" mode: the second (value) text of each line.
		mutable std::optional<sf::Text> valueText;
	};
}
