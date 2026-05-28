#include "Element.h"

namespace UI
{
	void Element::Draw(sf::RenderTarget& target, sf::Vector2f parentPosition, sf::Vector2f parentSize) const
	{
		const sf::Vector2f absolutePosition = ComputePosition(parentPosition, parentSize);

		DrawSelf(target, absolutePosition);

		for (const auto& child : children)
			child->Draw(target, absolutePosition, size);
	}

	void Element::Update(float deltaTime)
	{
		for (const auto& child : children)
			child->Update(deltaTime);
	}

	void Element::HandleEvent(const sf::Event& event)
	{
		for (const auto& child : children)
			child->HandleEvent(event);
	}

	Element& Element::AddChild(std::unique_ptr<Element> child)
	{
		children.push_back(std::move(child));
		return *children.back();
	}

	sf::Vector2f Element::ComputePosition(sf::Vector2f parentPosition, sf::Vector2f parentSize) const
	{
		return parentPosition
			+ sf::Vector2f(anchor.x * parentSize.x, anchor.y * parentSize.y)
			+ offset
			- sf::Vector2f(pivot.x * size.x, pivot.y * size.y);
	}

	void Element::DrawSelf(sf::RenderTarget& target, sf::Vector2f absolutePosition) const
	{}
}