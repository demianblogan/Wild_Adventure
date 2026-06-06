#include "Button.h"

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

	void Button::SetBackground(InteractionState state, std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		backgrounds[StateToIndex(state)] = &added;
		RefreshVisibility();
	}

	void Button::SetForeground(InteractionState state, std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		foregrounds[StateToIndex(state)] = &added;
		RefreshVisibility();
	}

	void Button::SetForegroundColor(InteractionState state, sf::Color color)
	{
		foregroundColors[StateToIndex(state)] = color;
		RefreshVisibility();
	}

	void Button::SetBackgroundTint(sf::Color color)
	{
		for (Element* background : backgrounds)
			if (background != nullptr)
				background->SetColor(color);
	}

	void Button::OnStateChanged()
	{
		RefreshVisibility();
	}

	void Button::RefreshVisibility()
	{
		Element* activeBackground = GetVariant(backgrounds, state);
		Element* activeForeground = GetVariant(foregrounds, state);

		for (Element* background : backgrounds)
			if (background != nullptr)
				background->isVisible = (background == activeBackground);

		for (Element* foreground : foregrounds)
			if (foreground != nullptr)
				foreground->isVisible = (foreground == activeForeground);

		if (activeForeground != nullptr)
		{
			const std::optional<sf::Color>& stateColor = foregroundColors[StateToIndex(state)];
			if (stateColor.has_value())
				activeForeground->SetColor(stateColor.value());
		}
	}

	Element* Button::GetVariant(const std::array<Element*, INTERACTION_STATE_COUNT>& variants, InteractionState state) const
	{
		Element* variant = variants[StateToIndex(state)];

		return variant ? variant : variants[StateToIndex(InteractionState::Normal)];
	}
}