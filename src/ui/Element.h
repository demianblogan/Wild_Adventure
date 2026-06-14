#pragma once

#include "ui/Animation.h"

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

#include <memory>
#include <string>
#include <vector>

namespace sf
{
	class RenderTarget;
	class Event;
}

namespace UI
{
	class Element
	{
	public:
		virtual ~Element() = default;

		void Draw(sf::RenderTarget& target, sf::Vector2f parentPosition, sf::Vector2f parentSize) const;

		// Draws again at the position cached by the last Draw call — e.g. into
		// the bloom layer. Only valid after Draw has run this frame.
		void DrawCached(sf::RenderTarget& target) const;

		virtual void Update(float deltaTime);
		virtual void HandleEvent(const sf::Event& event);

		Element& AddChild(std::unique_ptr<Element> child);

		std::vector<Element*> GetChildren() const;

		Element* FindByName(const std::string& name);

		virtual bool IsInteractive() const { return false; }
		sf::Vector2f GetAbsolutePosition() const { return absolutePosition; }

		Animation& AddAnimation(std::unique_ptr<Animation> animation);

		virtual void SetColor(sf::Color color) {}

		std::string name;

		sf::Vector2f anchor = { 0.0f, 0.0f }; // anchor point in the parent (0..1 of its size)
		sf::Vector2f pivot = { 0.0f, 0.0f };  // point within this element aligned with the parent's anchor (0..1 of this element's size)
		sf::Vector2f offset = { 0.0f, 0.0f }; // additional pixel offset applied after positioning
		sf::Vector2f size = { 0.0f, 0.0f };   // element size in pixels

		bool isVisible = true;

		// When true, the element is redrawn into the bloom layer every frame
		// and gets a pulsing aura in its own colors (e.g. the golden title).
		bool glow = false;

		// When true, this element's interactive descendants form one navigation row
		// (left/right move between them, up/down leave the row).
		bool isNavigationRow = false;

	protected:
	protected:
		sf::Vector2f ComputePosition(sf::Vector2f parentPosition, sf::Vector2f parentSize) const;

		virtual void DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const;

		mutable sf::Vector2f absolutePosition;

		std::vector<std::unique_ptr<Element>> children;
		std::vector<std::unique_ptr<Animation>> animations;
	};
}