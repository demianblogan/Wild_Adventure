#include "Stepper.h"

#include <SFML/Graphics/Rect.hpp>

namespace UI
{
	namespace
	{
		constexpr float KeyboardPressFlash = 0.12f;
	}

	void Stepper::SetLeftArrowNormal(std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		leftArrowNormal = &added;
		Refresh();
	}

	void Stepper::SetLeftArrowPressed(std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		leftArrowPressed = &added;
		Refresh();
	}

	void Stepper::SetRightArrowNormal(std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		rightArrowNormal = &added;
		Refresh();
	}

	void Stepper::SetRightArrowPressed(std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		rightArrowPressed = &added;
		Refresh();
	}

	void Stepper::SetValueLabel(std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		valueLabel = &added;
		Refresh();
	}

	void Stepper::SetArrowColor(InteractionState state, sf::Color color)
	{
		arrowColors[StateToIndex(state)] = color;
		Refresh();
	}

	void Stepper::SetValueColor(sf::Color color)
	{
		valueColor = color;
		Refresh();
	}

	void Stepper::SetOnStepLeft(std::function<void()> callback)
	{
		onStepLeft = std::move(callback);
	}

	void Stepper::SetOnStepRight(std::function<void()> callback)
	{
		onStepRight = std::move(callback);
	}

	void Stepper::OnDragStart(sf::Vector2f mousePosition)
	{
		if (!isEnabled)
			return;

		pressedTimer = 0.0f;

		if (leftArrowNormal != nullptr)
		{
			const sf::FloatRect bounds(leftArrowNormal->GetAbsolutePosition(), leftArrowNormal->size);
			if (bounds.contains(mousePosition))
			{
				pressedArrow = PressedArrow::Left;
				Refresh();

				if (onStepLeft)
					onStepLeft();

				return;
			}
		}

		if (rightArrowNormal != nullptr)
		{
			const sf::FloatRect bounds(rightArrowNormal->GetAbsolutePosition(), rightArrowNormal->size);
			if (bounds.contains(mousePosition))
			{
				pressedArrow = PressedArrow::Right;
				Refresh();

				if (onStepRight)
					onStepRight();

				return;
			}
		}
	}

	void Stepper::OnDragEnd()
	{
		pressedArrow = PressedArrow::None;
		pressedTimer = 0.0f;
		Refresh();
	}

	void Stepper::OnNavigate(int direction)
	{
		if (!isEnabled)
			return;

		if (direction < 0)
		{
			pressedArrow = PressedArrow::Left;
			pressedTimer = KeyboardPressFlash;
			Refresh();

			if (onStepLeft)
				onStepLeft();
		}
		else if (direction > 0)
		{
			pressedArrow = PressedArrow::Right;
			pressedTimer = KeyboardPressFlash;
			Refresh();

			if (onStepRight)
				onStepRight();
		}
	}

	void Stepper::Update(float deltaTime)
	{
		if (pressedTimer > 0.0f)
		{
			pressedTimer -= deltaTime;
			if (pressedTimer <= 0.0f)
			{
				pressedTimer = 0.0f;
				pressedArrow = PressedArrow::None;
				Refresh();
			}
		}

		Element::Update(deltaTime);
	}

	void Stepper::OnStateChanged()
	{
		Refresh();
	}

	void Stepper::OnEnabledChanged()
	{
		Refresh();
	}

	void Stepper::Refresh()
	{
		const bool leftPressed = (pressedArrow == PressedArrow::Left);
		const bool rightPressed = (pressedArrow == PressedArrow::Right);

		// Show the pressed image only for the arrow currently being clicked.
		if (leftArrowNormal != nullptr)
			leftArrowNormal->isVisible = !leftPressed;
		if (leftArrowPressed != nullptr)
			leftArrowPressed->isVisible = leftPressed;
		if (rightArrowNormal != nullptr)
			rightArrowNormal->isVisible = !rightPressed;
		if (rightArrowPressed != nullptr)
			rightArrowPressed->isVisible = rightPressed;

		const sf::Color tint = CurrentArrowTint();
		if (leftArrowNormal != nullptr)
			leftArrowNormal->SetColor(tint);
		if (rightArrowNormal != nullptr)
			rightArrowNormal->SetColor(tint);

		if (valueLabel != nullptr)
			valueLabel->SetColor(isEnabled ? valueColor : disabledColor);
	}

	sf::Color Stepper::CurrentArrowTint() const
	{
		if (!isEnabled)
			return disabledColor;

		// Normal when idle, brighter once focused/hovered. A whole-element Pressed
		// state (e.g. Enter) still counts as focused, so it stays bright.
		if (state == InteractionState::Normal)
			return arrowColors[StateToIndex(InteractionState::Normal)];

		return arrowColors[StateToIndex(InteractionState::Highlighted)];
	}
}