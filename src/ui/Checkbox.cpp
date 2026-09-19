#include "Checkbox.h"

namespace UI
{
	Checkbox::Checkbox()
	{
		SetOnPressed([this] { HandleRelease(); });
	}

	void Checkbox::SetBackground(InteractionState state, std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		backgrounds[StateToIndex(state)] = &added;
		RefreshVisibility();
	}

	void Checkbox::SetCheckedView(std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		checkedView = &added;
		RefreshVisibility();
	}

	void Checkbox::SetUncheckedView(std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		uncheckedView = &added;
		RefreshVisibility();
	}

	void Checkbox::SetViewColor(InteractionState state, sf::Color color)
	{
		viewColors[StateToIndex(state)] = color;
		RefreshVisibility();
	}

	void Checkbox::SetChecked(bool checked)
	{
		if (isChecked == checked)
			return;

		isChecked = checked;
		RefreshVisibility();

		if (onCheckedChanged)
			onCheckedChanged(isChecked);
	}

	void Checkbox::SetOnCheckedChanged(std::function<void(bool)> callback)
	{
		onCheckedChanged = std::move(callback);
	}

	void Checkbox::OnStateChanged()
	{
		RefreshVisibility();
	}

	void Checkbox::HandleRelease()
	{
		SetChecked(!isChecked);
	}

	void Checkbox::RefreshVisibility()
	{
		Element* activeBackground = GetBackgroundForState(state);

		for (Element* background : backgrounds)
			if (background != nullptr)
				background->isVisible = (background == activeBackground);

		if (checkedView != nullptr)
			checkedView->isVisible = isChecked;
		if (uncheckedView != nullptr)
			uncheckedView->isVisible = !isChecked;

		// Tint whichever view is shown, so the checkbox reflects focus/hover.
		Element* activeView = isChecked ? checkedView : uncheckedView;
		if (activeView != nullptr)
		{
			const std::optional<sf::Color>& tint = viewColors[StateToIndex(state)];
			if (tint.has_value())
				activeView->SetColor(tint.value());
		}
	}

	Element* Checkbox::GetBackgroundForState(InteractionState state) const
	{
		Element* background = backgrounds[StateToIndex(state)];

		return background != nullptr ? background : backgrounds[StateToIndex(InteractionState::Normal)];
	}
}