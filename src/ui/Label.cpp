#include "Label.h"

#include "core/Resources.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Angle.hpp>
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

		if (rotationDegrees == 0.0f)
		{
			// Fast, pixel-snapped path used by the overwhelming majority of
			// labels: anchors this element's absolutePosition to the glyph's
			// own ink top-left, unrotated.
			sf::Vector2f finalPosition = absolutePosition - bounds.position;

			finalPosition.x = std::floor(finalPosition.x);
			finalPosition.y = std::floor(finalPosition.y);

			drawableText.setOrigin({ 0.0f, 0.0f });
			drawableText.setRotation(sf::degrees(0.0f));
			drawableText.setPosition(finalPosition);
		}
		else
		{
			// Rotating pixel-snapped text always looks wrong at some angle no
			// matter which pixel it's snapped to, so this path skips the
			// floor() and instead pivots around the glyph's own visual
			// center, landing that center where absolutePosition's would be
			// if this were the unrotated, top-left-anchored case above.
			const sf::Vector2f center = bounds.position + bounds.size / 2.0f;

			drawableText.setOrigin(center);
			drawableText.setRotation(sf::degrees(rotationDegrees));
			drawableText.setPosition(absolutePosition + center);
		}

		target.draw(drawableText);
	}
}
