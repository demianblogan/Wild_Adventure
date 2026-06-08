#include "Root.h"

#include "core/VirtualScreen.h"
#include "ui/InteractiveElement.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Event.hpp>

#include <algorithm>

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
		focusRow = -1;
		focusColumn = -1;
		desiredColumn = 0;
		draggedElement = nullptr;
		activatedElement = nullptr;
		confirmHeld = false;

		rows.clear();

		if (content != nullptr)
			CollectInteractivesFrom(*content, nullptr);

		ResetFocus();
	}

	void Root::CollectInteractivesFrom(Element& element, std::vector<InteractiveElement*>* currentRow)
	{
		// An interactive element is a navigation leaf: we never descend into it,
		// so controls built from inner interactives are not collected separately.
		if (element.IsInteractive())
		{
			InteractiveElement* interactive = static_cast<InteractiveElement*>(&element);

			if (currentRow != nullptr)
				currentRow->push_back(interactive);
			else
				rows.push_back({ interactive });

			return;
		}

		// A row container groups all of its interactive descendants into one row.
		// Rows do not nest: a row flag found inside an existing row is ignored.
		if (element.isNavigationRow && currentRow == nullptr)
		{
			std::vector<InteractiveElement*> row;

			for (Element* child : element.GetChildren())
				CollectInteractivesFrom(*child, &row);

			if (!row.empty())
				rows.push_back(std::move(row));

			return;
		}

		for (Element* child : element.GetChildren())
			CollectInteractivesFrom(*child, currentRow);
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
			HandleMousePress();
			return;
		}

		if (event.getIf<sf::Event::MouseButtonReleased>())
		{
			HandleMouseRelease();
			return;
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

		for (int row = 0; row < static_cast<int>(rows.size()); row++)
		{
			for (int column = 0; column < static_cast<int>(rows[row].size()); column++)
			{
				InteractiveElement* element = rows[row][column];
				if (!element->IsEnabled())
					continue;

				const sf::FloatRect bounds(element->GetAbsolutePosition(), element->size);
				if (bounds.contains(mouse))
				{
					SetFocus(row, column);
					desiredColumn = column;
					return;
				}
			}
		}

		ClearFocus();
	}

	void Root::HandleMousePress()
	{
		InteractiveElement* element = CurrentElement();
		if (element == nullptr || !element->IsEnabled())
			return;

		element->Press();

		draggedElement = element;
		element->OnDragStart(virtualScreen.GetMousePosition());
	}

	void Root::HandleMouseRelease()
	{
		if (draggedElement != nullptr)
		{
			draggedElement->OnDragEnd();
			draggedElement->Release();
			draggedElement = nullptr;
		}
	}

	void Root::MoveRow(int direction)
	{
		if (rows.empty())
			return;

		const int rowCount = static_cast<int>(rows.size());
		int row = (focusRow >= 0) ? focusRow : ((direction > 0) ? -1 : rowCount);

		for (int step = 0; step < rowCount; step++)
		{
			row = (row + direction + rowCount) % rowCount;

			const int column = FirstEnabledColumn(row, desiredColumn);
			if (column >= 0)
			{
				SetFocus(row, column);
				return;
			}
		}
	}

	void Root::MoveColumn(int direction)
	{
		if (focusRow < 0 || focusRow >= static_cast<int>(rows.size()))
			return;

		const std::vector<InteractiveElement*>& cells = rows[focusRow];

		// Step toward the row edge, skipping disabled cells. No wrap inside a row.
		for (int next = focusColumn + direction; next >= 0 && next < static_cast<int>(cells.size()); next += direction)
		{
			if (cells[next]->IsEnabled())
			{
				SetFocus(focusRow, next);
				desiredColumn = next;
				return;
			}
		}
	}

	void Root::HandleConfirm(bool pressed)
	{
		InteractiveElement* element = CurrentElement();
		if (element == nullptr || !element->IsEnabled())
			return;

		// Value controls (slider/stepper) ignore Confirm: they are adjusted with
		// left/right while focused, never "entered".
		if (element->IsValueControl())
			return;

		if (pressed)
			element->Press();
		else
			element->Release(); // fires the button action
	}

	void Root::DeactivateFocused()
	{
		if (activatedElement != nullptr)
		{
			activatedElement->Deactivate();
			activatedElement = nullptr;
		}
	}

	InteractiveElement* Root::CurrentElement() const
	{
		if (focusRow < 0 || focusRow >= static_cast<int>(rows.size()))
			return nullptr;
		if (focusColumn < 0 || focusColumn >= static_cast<int>(rows[focusRow].size()))
			return nullptr;

		return rows[focusRow][focusColumn];
	}

	int Root::FirstEnabledColumn(int row, int preferredColumn) const
	{
		if (row < 0 || row >= static_cast<int>(rows.size()))
			return -1;

		const std::vector<InteractiveElement*>& cells = rows[row];
		if (cells.empty())
			return -1;

		const int clamped = std::clamp(preferredColumn, 0, static_cast<int>(cells.size()) - 1);
		if (cells[clamped]->IsEnabled())
			return clamped;

		// Search outward from the preferred column for the nearest enabled cell.
		for (int distance = 1; distance < static_cast<int>(cells.size()); distance++)
		{
			const int right = clamped + distance;
			const int left = clamped - distance;

			if (right < static_cast<int>(cells.size()) && cells[right]->IsEnabled())
				return right;
			if (left >= 0 && cells[left]->IsEnabled())
				return left;
		}

		return -1;
	}

	void Root::SetFocus(int row, int column, bool playSound)
	{
		if (row == focusRow && column == focusColumn)
			return;

		if (InteractiveElement* previous = CurrentElement())
		{
			previous->ClearPressed(); // a held-confirm press must not stick to the old element
			previous->SetHighlighted(false);

			if (previous == activatedElement)
				DeactivateFocused();
		}

		focusRow = row;
		focusColumn = column;

		if (InteractiveElement* current = CurrentElement())
		{
			current->SetHighlighted(true, playSound);

			// A focused value control auto-activates so left/right adjust it and
			// it shows its selected visual without a separate "enter" step.
			if (current->IsValueControl())
			{
				current->Activate();
				activatedElement = current;
			}
			else if (confirmHeld)
			{
				// Confirm is still held: the pressed visual follows the focus.
				current->Press();
			}
		}
	}

	void Root::ClearFocus()
	{
		if (InteractiveElement* current = CurrentElement())
		{
			current->ClearPressed();
			current->SetHighlighted(false);

			if (current == activatedElement)
				DeactivateFocused();
		}

		focusRow = -1;
		focusColumn = -1;
	}

	void Root::ResetFocus()
	{
		ClearFocus();

		for (int row = 0; row < static_cast<int>(rows.size()); row++)
		{
			const int column = FirstEnabledColumn(row, 0);
			if (column >= 0)
			{
				SetFocus(row, column, false);
				desiredColumn = column;
				return;
			}
		}
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

	void Root::NavigateUp()
	{
		activeMode = InputMode::Selection;
		MoveRow(-1);
	}

	void Root::NavigateDown()
	{
		activeMode = InputMode::Selection;
		MoveRow(1);
	}

	void Root::NavigateLeft()
	{
		activeMode = InputMode::Selection;

		InteractiveElement* element = CurrentElement();
		if (element != nullptr && element->IsEnabled() && element->IsValueControl())
			element->OnNavigate(-1);
		else
			MoveColumn(-1);
	}

	void Root::NavigateRight()
	{
		activeMode = InputMode::Selection;

		InteractiveElement* element = CurrentElement();
		if (element != nullptr && element->IsEnabled() && element->IsValueControl())
			element->OnNavigate(1);
		else
			MoveColumn(1);
	}

	void Root::Confirm(bool pressed)
	{
		activeMode = InputMode::Selection;
		confirmHeld = pressed;
		HandleConfirm(pressed);
	}
}