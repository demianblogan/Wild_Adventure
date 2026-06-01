#include "TextField.h"

#include "ui/Image.h"
#include "ui/Label.h"

#include <SFML/Window/Event.hpp>

namespace UI
{
	namespace
	{
		int StateToIndex(InteractionState state)
		{
			switch (state)
			{
			case InteractionState::Normal:
				return 0;
			case InteractionState::Highlighted:
				return 1;
			case InteractionState::Pressed:
				return 2;
			default:
				return 0;
			}
		}
	}

	TextField::TextField(Resources& resources, const std::string& fontName)
		: resources(resources)
		, fontName(fontName)
	{
		auto labelPtr = std::make_unique<Label>(resources, fontName);
		labelPtr->anchor = { 0.0f, 0.5f };
		labelPtr->pivot = { 0.0f, 0.5f };
		labelPtr->offset = { 4.0f, 0.0f };
		labelPtr->SetCharacterSize(16);
		labelPtr->SetColor(sf::Color::White);
		textLabel = static_cast<Label*>(&AddChild(std::move(labelPtr)));

		auto cursorPtr = std::make_unique<Image>(resources);
		cursorPtr->size = { 1.0f, 16.0f };
		cursorPtr->anchor = { 0.0f, 0.5f };
		cursorPtr->pivot = { 0.0f, 0.5f };
		cursorPtr->SetColor(sf::Color::White);
		cursorPtr->isVisible = false;
		cursor = static_cast<Image*>(&AddChild(std::move(cursorPtr)));
	}

	void TextField::SetBackground(InteractionState state, std::unique_ptr<Element> element)
	{
		Element& added = AddChildBehind(std::move(element));
		backgrounds[StateToIndex(state)] = &added;
		RefreshBackgroundVisibility();
	}

	void TextField::SetText(const std::string& newText)
	{
		text = newText;
		cursorPosition = text.size();
		RefreshTextDisplay();
		RefreshCursorPosition();
		NotifyChanged();
	}

	void TextField::SetFilter(std::function<bool(char32_t)> filter)
	{
		this->filter = std::move(filter);
	}

	void TextField::SetOnTextChanged(std::function<void(const std::string&)> callback)
	{
		onTextChanged = std::move(callback);
	}

	void TextField::Update(float deltaTime)
	{
		Element::Update(deltaTime);

		if (!isActivated)
			return;

		blinkTimer += deltaTime;
		if (blinkTimer >= blinkPeriod)
		{
			blinkTimer -= blinkPeriod;
			cursorBlinkVisible = !cursorBlinkVisible;

			if (cursor != nullptr)
				cursor->isVisible = cursorBlinkVisible;
		}
	}

	void TextField::HandleEvent(const sf::Event& event)
	{
		if (isActivated)
		{
			if (const auto* entered = event.getIf<sf::Event::TextEntered>())
			{
				const char32_t character = entered->unicode;

				if (character == 8) // backspace
				{
					DeleteBefore();
				}
				else if (character == 127) // delete
				{
					DeleteAfter();
				}
				else if (character >= 32) // printable characters
				{
					if (!filter || filter(character))
						InsertCharacter(character);
				}
			}
			else if (const auto* key = event.getIf<sf::Event::KeyPressed>())
			{
				if (key->code == sf::Keyboard::Key::Left)
					MoveCursorLeft();
				else if (key->code == sf::Keyboard::Key::Right)
					MoveCursorRight();
				else if (key->code == sf::Keyboard::Key::Delete)
					DeleteAfter();
			}
		}

		Element::HandleEvent(event);
	}

	void TextField::OnStateChanged()
	{
		RefreshBackgroundVisibility();
	}

	void TextField::OnActivated()
	{
		blinkTimer = 0.0f;
		cursorBlinkVisible = true;
		if (cursor != nullptr)
			cursor->isVisible = true;
		RefreshCursorPosition();
	}

	void TextField::OnDeactivated()
	{
		if (cursor != nullptr)
			cursor->isVisible = false;
	}

	void TextField::InsertCharacter(char32_t character)
	{
		text.insert(cursorPosition, 1, static_cast<char>(character));
		cursorPosition++;
		RefreshTextDisplay();
		RefreshCursorPosition();
		NotifyChanged();
	}

	void TextField::DeleteBefore()
	{
		if (cursorPosition > 0)
		{
			text.erase(cursorPosition - 1, 1);
			cursorPosition--;
			RefreshTextDisplay();
			RefreshCursorPosition();
			NotifyChanged();
		}
	}

	void TextField::DeleteAfter()
	{
		if (cursorPosition < text.size())
		{
			text.erase(cursorPosition, 1);
			RefreshTextDisplay();
			NotifyChanged();
		}
	}

	void TextField::MoveCursorLeft()
	{
		if (cursorPosition > 0)
		{
			cursorPosition--;
			RefreshCursorPosition();
		}
	}

	void TextField::MoveCursorRight()
	{
		if (cursorPosition < text.size())
		{
			cursorPosition++;
			RefreshCursorPosition();
		}
	}

	void TextField::RefreshTextDisplay()
	{
		if (textLabel != nullptr)
			textLabel->SetText(text);
	}

	void TextField::RefreshCursorPosition()
	{
		if (cursor == nullptr || textLabel == nullptr)
			return;

		const std::string prefix = text.substr(0, cursorPosition);

		Label probe(resources, fontName);
		probe.SetCharacterSize(16);
		probe.SetText(prefix);
		const float prefixWidth = probe.size.x;

		cursor->offset = { 4.0f + prefixWidth, 0.0f };
	}

	void TextField::NotifyChanged()
	{
		if (onTextChanged)
			onTextChanged(text);
	}

	void TextField::RefreshBackgroundVisibility()
	{
		Element* activeBackground = GetBackgroundForState(state);

		for (Element* background : backgrounds)
			if (background != nullptr)
				background->isVisible = (background == activeBackground);
	}

	Element* TextField::GetBackgroundForState(InteractionState state) const
	{
		Element* background = backgrounds[StateToIndex(state)];
		if (background != nullptr)
			return background;
		else
			return backgrounds[StateToIndex(InteractionState::Normal)];
	}
}