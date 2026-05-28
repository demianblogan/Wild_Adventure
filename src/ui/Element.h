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

		sf::Vector2f anchor;
		sf::Vector2f pivot;
		sf::Vector2f offset;
		sf::Vector2f size;

	protected:
		sf::Vector2f ComputePosition(sf::Vector2f parentPosition, sf::Vector2f parentSize) const;

		virtual void DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const;

		std::vector<std::unique_ptr<Element>> children;
	};
}