#include "InteractiveElement.h"

#include "audio/Mixer.h"

namespace UI
{
	void InteractiveElement::SetHighlighted(bool highlighted, bool playSound)
	{
		if (state == InteractionState::Pressed)
			return;

		const InteractionState newState = highlighted ? InteractionState::Highlighted : InteractionState::Normal;

		if (newState != state)
		{
			const bool wasNotHighlighted = (state != InteractionState::Highlighted);
			state = newState;
			OnStateChanged();

			if (newState == InteractionState::Highlighted && wasNotHighlighted && playSound && onHighlighted)
				onHighlighted();
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

	void InteractiveElement::Activate()
	{
		if (!isActivated)
		{
			isActivated = true;
			OnActivated();
		}
	}

	void InteractiveElement::Deactivate()
	{
		if (isActivated)
		{
			isActivated = false;
			OnDeactivated();
		}
	}

	void InteractiveElement::SetOnPressed(std::function<void()> callback)
	{
		onPressed = std::move(callback);
	}
	void InteractiveElement::SetOnHighlighted(std::function<void()> callback)
	{
		onHighlighted = std::move(callback);
	}
}