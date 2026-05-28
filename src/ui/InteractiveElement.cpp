#include "InteractiveElement.h"

namespace UI
{
	void InteractiveElement::SetHighlighted(bool highlighted)
	{
		if (state == InteractionState::Pressed)
			return;

		const InteractionState newState = highlighted ? InteractionState::Highlighted : InteractionState::Normal;

		if (newState != state)
		{
			state = newState;
			OnStateChanged();
		}
	}

	void InteractiveElement::Press()
	{
		if (state != InteractionState::Pressed)
		{
			state = InteractionState::Pressed;
			OnStateChanged();
		}
	}

	void InteractiveElement::Release()
	{
		const bool wasPressed = (state == InteractionState::Pressed);

		if (wasPressed)
		{
			state = InteractionState::Highlighted;
			OnStateChanged();

			if (onPressed)
				onPressed();
		}
	}

	void InteractiveElement::SetOnPressed(std::function<void()> callback)
	{
		onPressed = std::move(callback);
	}
}