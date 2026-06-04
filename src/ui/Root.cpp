#include "Root.h"

#include "core/VirtualScreen.h"
#include "ui/InteractiveElement.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Event.hpp>

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

	Element* Root::FindByName(const std::string& name)
	{
		return content ? content->FindByName(name) : nullptr;
	}

	void Root::CollectInteractives()
	{
		highlightedIndex = -1;
		draggedElement = nullptr;
		activatedElement = nullptr;

		interactives.clear();

		if (content != nullptr)
			CollectInteractivesFrom(*content);

		if (!interactives.empty())
		{
			highlightedIndex = 0;
			interactives[0]->SetHighlighted(true, false);
		}
	}

	void Root::CollectInteractivesFrom(Element& element)
	{
		if (element.IsInteractive())
			interactives.push_back(static_cast<InteractiveElement*>(&element));

		for (Element* child : element.GetChildren())
			CollectInteractivesFrom(*child);
	}

	void Root::HandleEvent(const sf::Event& event)
	{
		if (event.getIf<sf::Event::MouseMoved>())
		{
			activeMode = InputMode::Cursor;
			HandleMouseMove();
			return;
		}

		if (event.getIf<sf::Event::MouseButtonPressed>())
		{
			activeMode = InputMode::Cursor;

			if (activatedElement != nullptr)
			{
				const sf::Vector2f mouse = virtualScreen.GetMousePosition();
				const sf::FloatRect activatedBounds(activatedElement->GetAbsolutePosition(), activatedElement->size);

				if (!activatedBounds.contains(mouse))
					DeactivateCurrent();
			}

			HandleMousePress();
			return;
		}

		if (event.getIf<sf::Event::MouseButtonReleased>())
		{
			HandleMouseRelease();
			return;
		}

		if (activatedElement != nullptr)
		{
			if (const auto* key = event.getIf<sf::Event::KeyPressed>())
			{
				if (key->code == sf::Keyboard::Key::Escape)
				{
					DeactivateCurrent();
					return;
				}
			}

			activatedElement->HandleEvent(event);
			return;
		}

		if (const auto* key = event.getIf<sf::Event::KeyPressed>())
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

		if (draggedElement != nullptr)
		{
			draggedElement->OnDragMove(mouse);
			return;
		}

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
		{
			InteractiveElement* element = interactives[highlightedIndex];
			element->Press();

			draggedElement = element;
			element->OnDragStart(virtualScreen.GetMousePosition());
		}
	}

	void Root::HandleMouseRelease()
	{
		if (draggedElement != nullptr)
		{
			draggedElement->OnDragEnd();
			draggedElement->Release();

			if (draggedElement->RequiresActivation() && !draggedElement->IsActivated())
			{
				activatedElement = draggedElement;
				draggedElement->Activate();
			}

			draggedElement = nullptr;
		}
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

		InteractiveElement* element = interactives[highlightedIndex];

		if (pressed)
		{
			element->Press();
		}
		else
		{
			element->Release();

			if (element->RequiresActivation() && !element->IsActivated())
			{
				activatedElement = element;
				element->Activate();
			}
		}
	}

	void Root::DeactivateCurrent()
	{
		if (activatedElement != nullptr)
		{
			activatedElement->Deactivate();
			activatedElement = nullptr;
		}
	}

	void Root::SetHighlightedIndex(int index)
	{
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