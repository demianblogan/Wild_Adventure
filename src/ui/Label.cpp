#include "Label.h"

#include "core/Resources.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/String.hpp>

#include <algorithm>
#include <cmath>

namespace UI
{
	Label::Label(Resources& resources, const std::string& fontName)
		: drawableText(resources.fonts.Get(fontName))
	{
		drawableText.setCharacterSize(16); // matches this project's old Label default
	}

	void Label::SetText(const std::string& text)
	{
		// text is UTF-8 (JSON literals and every localization catalog are
		// UTF-8); sf::Text::setString(std::string) instead assumes ANSI and
		// mangles anything outside plain ASCII, so it has to go through
		// fromUtf8 explicitly.
		drawableText.setString(sf::String::fromUtf8(text.begin(), text.end()));
		RecalculateSize();
	}

	void Label::SetCharacterSize(unsigned int characterSize)
	{
		drawableText.setCharacterSize(characterSize);
		RecalculateSize();
	}

	void Label::SetColor(sf::Color color)
	{
		this->color = color;
	}

	void Label::SetAlpha(float alpha)
	{
		const std::uint8_t byteAlpha = static_cast<std::uint8_t>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f);
		color.a = byteAlpha;
		outlineColor.a = byteAlpha;
	}

	void Label::SetOutlineColor(sf::Color color)
	{
		outlineColor = color;
	}

	void Label::SetOutlineThickness(float thickness)
	{
		outlineThickness = thickness;
		RecalculateSize();
	}

	void Label::RecalculateSize()
	{
		drawableText.setOutlineThickness(outlineThickness);

		const sf::FloatRect bounds = drawableText.getLocalBounds();
		size = { std::ceil(bounds.size.x), std::ceil(bounds.size.y) };
	}

	void Label::DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const
	{
		drawableText.setFillColor(color);
		drawableText.setOutlineColor(outlineColor);

		const sf::FloatRect bounds = drawableText.getLocalBounds();
		sf::Vector2f finalPosition = absolutePosition - bounds.position;

		finalPosition.x = std::floor(finalPosition.x);
		finalPosition.y = std::floor(finalPosition.y);

		drawableText.setPosition(finalPosition);

		target.draw(drawableText);
	}
}
