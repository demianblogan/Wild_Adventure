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

	void Element::DrawCached(sf::RenderTarget& target) const
	{
		if (!isVisible)
			return;

		DrawSelf(target, absolutePosition);

		for (const auto& child : children)
			child->Draw(target, absolutePosition, size);
	}

	void Element::Update(float deltaTime)
	{
		// Index-based on purpose: an animation's OnFinished callback may add a
		// new animation to this same element (AddAnimation push_back()s into
		// this vector), which can reallocate it. A range-based for loop would
		// be left holding iterators into freed memory; re-checking
		// animations.size() each pass keeps this safe either way.
		for (std::size_t i = 0; i < animations.size(); i++)
			animations[i]->Update(deltaTime);

		animations.erase(
			std::remove_if(animations.begin(), animations.end(),
				[](const std::unique_ptr<Animation>& a) { return a->IsFinished(); }),
			animations.end());

		// Same reasoning as the animations loop above: an OnFinished callback
		// may add a new child to this element (e.g. a one-shot VFX spawned
		// when a landing animation completes), which can reallocate `children`.
		for (std::size_t i = 0; i < children.size(); i++)
			children[i]->Update(deltaTime);
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

	std::vector<Element*> Element::GetChildren() const
	{
		std::vector<Element*> result;
		result.reserve(children.size());

		for (const auto& child : children)
			result.push_back(child.get());

		return result;
	}

	Element* Element::FindByName(const std::string& targetName)
	{
		if (name == targetName)
			return this;

		for (const auto& child : children)
		{
			if (Element* found = child->FindByName(targetName))
				return found;
		}

		return nullptr;
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

	void Element::DrawSelf(sf::RenderTarget&, sf::Vector2f) const
	{}
}