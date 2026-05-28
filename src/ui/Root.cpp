#include "Root.h"

#include "core/VirtualScreen.h"
#include "ui/InteractiveElement.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>

namespace UI
{
	Root::Root(VirtualScreen& virtualScreen)
		: virtualScreen(virtualScreen)
	{}

	Element& Root::SetContent(std::unique_ptr<Element> newContent)
	{
		content = std::move(newContent);
		CollectInteractives();
		return *content;
	}

	void Root::CollectInteractives()
	{
		interactives.clear();
		if (content)
			CollectFrom(*content);
	}

	void Root::CollectFrom(Element& element)
	{
		if (element.IsInteractive())
		{
			auto* ie = static_cast<InteractiveElement*>(&element);
			std::cout << "collected interactive at offset.y = " << ie->offset.y << "\n";
			interactives.push_back(ie);
		}

		for (Element* child : element.GetChildren())
			CollectFrom(*child);
	}

	void Root::HandleEvent(const sf::Event& event)
	{
		if (event.getIf<sf::Event::MouseMoved>())
		{
			activeMode = InputMode::Cursor;
			HandleMouseMove();
		}
		else if (event.getIf<sf::Event::MouseButtonPressed>())
		{
			activeMode = InputMode::Cursor;
			HandleMousePress();
		}
		else if (event.getIf<sf::Event::MouseButtonReleased>())
		{
			HandleMouseRelease();
		}
		else if (const auto* key = event.getIf<sf::Event::KeyPressed>())
		{
			if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::Right)
			{
				activeMode = InputMode::Selection;
				HandleNavigation(1);
			}
			else if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::Left)
			{
				activeMode = InputMode::Selection;
				HandleNavigation(-1);
			}
			else if (key->code == sf::Keyboard::Key::Enter)
			{
				activeMode = InputMode::Selection;
				HandleConfirm(true);
			}
		}
		else if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>())
		{
			if (keyReleased->code == sf::Keyboard::Key::Enter)
				HandleConfirm(false);
		}
	}

	void Root::HandleMouseMove()
	{
		const sf::Vector2f mouse = virtualScreen.GetMousePosition();

		int foundIndex = -1;

		for (int i = 0; i < static_cast<int>(interactives.size()); i++)
		{
			const sf::FloatRect bounds(interactives[i]->GetAbsolutePosition(), interactives[i]->size);

			if (bounds.contains(mouse))
			{
				foundIndex = i;
				break;
			}
		}

		SetHighlightedIndex(foundIndex);
	}

	void Root::HandleMousePress()
	{
		if (highlightedIndex >= 0)
			interactives[highlightedIndex]->Press();
	}

	void Root::HandleMouseRelease()
	{
		if (highlightedIndex >= 0)
			interactives[highlightedIndex]->Release();
	}

	void Root::HandleNavigation(int direction)
	{
		if (interactives.empty())
			return;

		int newIndex = highlightedIndex;

		if (newIndex < 0)
			newIndex = (direction > 0) ? 0 : static_cast<int>(interactives.size()) - 1;
		else
			newIndex = (newIndex + direction + static_cast<int>(interactives.size()))
			% static_cast<int>(interactives.size());

		SetHighlightedIndex(newIndex);
	}

	void Root::HandleConfirm(bool pressed)
	{
		if (highlightedIndex < 0)
			return;

		if (pressed)
			interactives[highlightedIndex]->Press();
		else
			interactives[highlightedIndex]->Release();
	}

	void Root::SetHighlightedIndex(int index)
	{
		//std::cout << "highlight index: " << index << " (total " << interactives.size() << ")\n";

		if (index == highlightedIndex)
			return;

		if (highlightedIndex >= 0)
			interactives[highlightedIndex]->SetHighlighted(false);

		highlightedIndex = index;

		if (highlightedIndex >= 0)
			interactives[highlightedIndex]->SetHighlighted(true);
	}

	void Root::Update(float deltaTime)
	{
		if (content)
			content->Update(deltaTime);
	}

	void Root::Draw(sf::RenderTarget& target) const
	{
		if (content)
			content->Draw(target, { 0.0f, 0.0f },
				{ static_cast<float>(VirtualScreen::WIDTH), static_cast<float>(VirtualScreen::HEIGHT) });
	}
}