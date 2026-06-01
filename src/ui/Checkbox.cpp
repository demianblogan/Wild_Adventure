#include "Checkbox.h"

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
	}

	Element* Checkbox::GetBackgroundForState(InteractionState state) const
	{
		Element* background = backgrounds[StateToIndex(state)];

		return background != nullptr ? background : backgrounds[StateToIndex(InteractionState::Normal)];
	}
}