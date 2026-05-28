#pragma once

#include "ui/Element.h"

#include <SFML/System/Vector2.hpp>

#include <memory>
#include <vector>

namespace sf
{
	class RenderTarget;
	class Event;
}

class VirtualScreen;

namespace UI
{
	class InteractiveElement;

	enum class InputMode
	{
		Cursor,
		Selection
	};

	class Root
	{
	public:
		Root(VirtualScreen& virtualScreen);

		Element& SetContent(std::unique_ptr<Element> content);

		void HandleEvent(const sf::Event& event);
		void Update(float deltaTime);
		void Draw(sf::RenderTarget& target) const;

	private:
		void CollectInteractives();
		void CollectFrom(Element& element);

		void HandleMouseMove();
		void HandleMousePress();
		void HandleMouseRelease();
		void HandleNavigation(int direction);
		void HandleConfirm(bool pressed);

		void SetHighlightedIndex(int index);

		VirtualScreen& virtualScreen;
		std::unique_ptr<Element> content;
		std::vector<InteractiveElement*> interactives;

		InputMode activeMode = InputMode::Cursor;
		int highlightedIndex = -1;
	};
}