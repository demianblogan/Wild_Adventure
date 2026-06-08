#pragma once

#include "ui/Element.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <optional>
#include <string>

struct Resources;

namespace sf
{
	class RenderTarget;
}

namespace UI
{
	class Image : public Element
	{
	public:
		Image(Resources& resources);

		void SetTexture(const std::string& textureName);
		void SetTextureRect(sf::IntRect rect); // sub-rectangle for spritesheets
		void SetColor(sf::Color color) override;
		void SetAlpha(float alpha);

		// 9-slice borders in source-texture pixels; zero on all sides = plain stretch.
		void SetBorder(float left, float top, float right, float bottom);

	protected:
		void DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const override;

	private:
		void DrawStretched(sf::RenderTarget& target, sf::Vector2f absolutePosition) const;
		void DrawNineSlice(sf::RenderTarget& target, sf::Vector2f absolutePosition) const;

		bool HasBorder() const;

		Resources& resources;
		std::string textureName;
		std::optional<sf::IntRect> textureRect;
		sf::Color color = sf::Color::White;

		float borderLeft = 0.0f;
		float borderTop = 0.0f;
		float borderRight = 0.0f;
		float borderBottom = 0.0f;
	};
}