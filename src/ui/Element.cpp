#include "Element.h"

#include <algorithm>

namespace UI
{
	void Element::Draw(sf::RenderTarget& target, sf::Vector2f parentPosition, sf::Vector2f parentSize) const
	{
		if (!isVisible)
			return;

		absolutePosition = ComputePosition(parentPosition, parentSize);

		DrawSelf(target, absolutePosition);

		for (const auto& child : children)
			child->Draw(target, absolutePosition, size);
	}

	void Element::Update(float deltaTime)
	{
		for (const auto& animation : animations)
			animation->Update(deltaTime);

		animations.erase(
			std::remove_if(animations.begin(), animations.end(),
				[](const std::unique_ptr<Animation>& a) { return a->IsFinished(); }),
			animations.end());

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

	Element& Element::AddChildBack(std::unique_ptr<Element> child)
	{
		children.insert(children.begin(), std::move(child));

		return *children.front();
	}

	std::vector<Element*> Element::GetChildren() const
	{
		std::vector<Element*> result;
		result.reserve(children.size());

		for (const auto& child : children)
			result.push_back(child.get());

		return result;
	}

	Animation& Element::AddAnimation(std::unique_ptr<Animation> animation)
	{
		animations.push_back(std::move(animation));
		return *animations.back();
	}

	void Element::ClearAnimations()
	{
		animations.clear();
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