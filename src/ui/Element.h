#pragma once

#include <SFML/System/Vector2.hpp>

#include <memory>
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
		virtual void Update(float deltaTime);
		virtual void HandleEvent(const sf::Event& event);

		Element& AddChild(std::unique_ptr<Element> child);
		std::vector<Element*> GetChildren() const;

		virtual bool IsInteractive() const { return false; }
		sf::Vector2f GetAbsolutePosition() const { return absolutePosition; }

		sf::Vector2f anchor;
		sf::Vector2f pivot;
		sf::Vector2f offset;
		sf::Vector2f size;

		bool isVisible = true;

	protected:
		sf::Vector2f ComputePosition(sf::Vector2f parentPosition, sf::Vector2f parentSize) const;

		virtual void DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const;

		mutable sf::Vector2f absolutePosition;

		std::vector<std::unique_ptr<Element>> children;
	};
}