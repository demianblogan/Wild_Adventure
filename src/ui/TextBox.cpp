#include "TextBox.h"

#include "core/Resources.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>
#include <sstream>

namespace UI
{
	namespace
	{
		// Gap between the "name" and "value" halves of a "name\tvalue" line.
		constexpr float LinkValueGap = 4.0f;
	}

	TextBox::TextBox(Resources& resources, const std::string& fontName)
		: resources(resources)
		, fontName(fontName)
	{}

	void TextBox::SetText(const std::string& text)
	{
		this->text = text;
		Rebuild();
	}

	void TextBox::SetCharacterSize(unsigned int characterSize)
	{
		this->characterSize = characterSize;
		Rebuild();
	}

	void TextBox::SetColor(sf::Color color)
	{
		this->color = color;
		ApplyColors();
	}

	void TextBox::SetOutlineColor(sf::Color color)
	{
		outlineColor = color;
		ApplyColors();
	}

	void TextBox::SetOutlineThickness(float thickness)
	{
		// Outline thickness can shift where words wrap, so this one needs the
		// full re-wrap rather than just re-coloring the cached lines.
		outlineThickness = thickness;
		Rebuild();
	}

	void TextBox::SetAlignment(Alignment alignment)
	{
		this->alignment = alignment;
	}

	void TextBox::SetSecondColor(sf::Color color)
	{
		secondColor = color;
		ApplyColors();
	}

	void TextBox::SetSecondOutlineColor(sf::Color color)
	{
		secondOutlineColor = color;
		ApplyColors();
	}

	void TextBox::ApplyColors()
	{
		// In "name\tvalue" mode, lineTexts[0] is the name half (primary
		// color) and valueText is the value half (second color); otherwise
		// lineTexts holds the wrapped body lines, all in the primary color.
		for (sf::Text& lineText : lineTexts)
		{
			lineText.setFillColor(color);
			lineText.setOutlineColor(outlineColor);
		}

		if (valueText.has_value())
		{
			valueText->setFillColor(secondColor);
			valueText->setOutlineColor(secondOutlineColor);
		}
	}

	void TextBox::Rebuild()
	{
		lineTexts.clear();
		valueText.reset();

		const sf::Font& font = resources.fonts.Get(fontName);
		const float lineSpacing = font.getLineSpacing(characterSize);

		const std::size_t tabPosition = text.find('\t');
		if (tabPosition != std::string::npos)
		{
			sf::Text nameText(font, text.substr(0, tabPosition), characterSize);
			nameText.setFillColor(color);
			nameText.setOutlineColor(outlineColor);
			nameText.setOutlineThickness(outlineThickness);
			lineTexts.push_back(std::move(nameText));

			sf::Text value(font, text.substr(tabPosition + 1), characterSize);
			value.setFillColor(secondColor);
			value.setOutlineColor(secondOutlineColor);
			value.setOutlineThickness(outlineThickness);
			valueText = std::move(value);

			size.y = lineSpacing;
			return;
		}

		std::vector<std::string> wrappedLines;
		std::istringstream words(text);
		std::string word;
		std::string currentLine;

		while (words >> word)
		{
			const std::string candidate = currentLine.empty() ? word : currentLine + " " + word;

			sf::Text probe(font, candidate, characterSize);
			probe.setOutlineThickness(outlineThickness);

			if (size.x > 0.0f && probe.getLocalBounds().size.x > size.x && !currentLine.empty())
			{
				wrappedLines.push_back(currentLine);
				currentLine = word;
			}
			else
			{
				currentLine = candidate;
			}
		}

		if (!currentLine.empty())
			wrappedLines.push_back(currentLine);

		for (const std::string& line : wrappedLines)
		{
			sf::Text lineText(font, line, characterSize);
			lineText.setFillColor(color);
			lineText.setOutlineColor(outlineColor);
			lineText.setOutlineThickness(outlineThickness);
			lineTexts.push_back(std::move(lineText));
		}

		size.y = wrappedLines.empty() ? 0.0f : lineSpacing * static_cast<float>(wrappedLines.size());
	}

	void TextBox::DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const
	{
		if (valueText.has_value())
		{
			sf::Text& nameText = lineTexts[0];
			const sf::FloatRect nameBounds = nameText.getLocalBounds();
			const sf::FloatRect valueBounds = valueText->getLocalBounds();
			const float combinedWidth = nameBounds.size.x + LinkValueGap + valueBounds.size.x;

			float x = absolutePosition.x;
			switch (alignment)
			{
			case Alignment::Center:
				x += (size.x - combinedWidth) * 0.5f;
				break;
			case Alignment::Right:
				x += size.x - combinedWidth;
				break;
			case Alignment::Left:
			default:
				break;
			}

			const float nameX = x - nameBounds.position.x;
			nameText.setPosition({ std::floor(nameX), std::floor(absolutePosition.y) });
			target.draw(nameText);

			const float valueX = x + nameBounds.size.x + LinkValueGap - valueBounds.position.x;
			valueText->setPosition({ std::floor(valueX), std::floor(absolutePosition.y) });
			target.draw(*valueText);
			return;
		}

		const sf::Font& font = resources.fonts.Get(fontName);
		const float lineSpacing = font.getLineSpacing(characterSize);

		float y = absolutePosition.y;
		for (sf::Text& lineText : lineTexts)
		{
			const sf::FloatRect bounds = lineText.getLocalBounds();

			float x = absolutePosition.x;
			switch (alignment)
			{
			case Alignment::Center:
				x += (size.x - bounds.size.x) * 0.5f;
				break;
			case Alignment::Right:
				x += size.x - bounds.size.x;
				break;
			case Alignment::Left:
			default:
				break;
			}
			x -= bounds.position.x;

			// Deliberately no per-line vertical bounds correction: using the
			// font's uniform line-spacing metric for every row (rather than
			// each line's own ink bounds) is what keeps row spacing even
			// regardless of ascenders/descenders in any particular line.
			lineText.setPosition({ std::floor(x), std::floor(y) });

			target.draw(lineText);

			y += lineSpacing;
		}
	}
}
