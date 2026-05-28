#include "Label.h"

#include "core/Resources.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

namespace UI
{
	Label::Label(Resources& resources, const std::string& fontName)
		: resources(resources)
		, fontName(fontName)
	{}

	void Label::SetText(const std::string& text)
	{
		this->text = text;
		RecalculateSize();
	}

	void Label::SetCharacterSize(unsigned int characterSize)
	{
		this->characterSize = characterSize;
		RecalculateSize();
	}

	void Label::SetColor(sf::Color color)
	{
		this->color = color;
	}

	void Label::RecalculateSize()
	{
		const sf::Font& font = resources.fonts.Get(fontName);

		sf::Text measuredText(font, text, characterSize);
		const sf::FloatRect bounds = measuredText.getLocalBounds();

		size = { bounds.size.x, bounds.size.y };
	}

	void Label::DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const
	{
		const sf::Font& font = resources.fonts.Get(fontName);

		sf::Text drawableText(font, text, characterSize);
		drawableText.setFillColor(color);

		const sf::FloatRect bounds = drawableText.getLocalBounds();
		drawableText.setPosition(absolutePosition - bounds.position);

		target.draw(drawableText);
	}
}