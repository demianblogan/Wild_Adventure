#include "Slider.h"

#include <SFML/Window/Event.hpp>

#include <algorithm>

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

	void Slider::SetTrack(std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		track = &added;
	}

	void Slider::SetFill(std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		fill = &added;
		RefreshLayout();
	}

	void Slider::SetHandle(InteractionState state, std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		handles[StateToIndex(state)] = &added;
		RefreshHandleVisibility();
		RefreshLayout();
	}

	void Slider::SetActivatedHandle(std::unique_ptr<Element> element)
	{
		Element& added = AddChild(std::move(element));
		activatedHandle = &added;
		RefreshHandleVisibility();
		RefreshLayout();
	}

	void Slider::SetRange(float minValue, float maxValue)
	{
		this->minValue = minValue;
		this->maxValue = maxValue;
		SetValue(currentValue);
	}

	void Slider::SetStep(float step)
	{
		this->step = step;
	}

	void Slider::SetValue(float value)
	{
		const float clamped = std::clamp(value, minValue, maxValue);

		if (clamped == currentValue)
			return;

		currentValue = clamped;
		RefreshLayout();

		if (onValueChanged)
			onValueChanged(currentValue);
	}

	void Slider::SetOnValueChanged(std::function<void(float)> callback)
	{
		onValueChanged = std::move(callback);
	}

	void Slider::OnStateChanged()
	{
		RefreshHandleVisibility();
	}

	void Slider::OnActivated()
	{
		RefreshHandleVisibility();
	}

	void Slider::OnDeactivated()
	{
		RefreshHandleVisibility();
	}

	void Slider::OnDragStart(sf::Vector2f mousePosition)
	{
		SetValueFromMouseX(mousePosition.x);
	}

	void Slider::OnDragMove(sf::Vector2f mousePosition)
	{
		SetValueFromMouseX(mousePosition.x);
	}

	void Slider::HandleEvent(const sf::Event& event)
	{
		if (isActivated)
		{
			if (const auto* key = event.getIf<sf::Event::KeyPressed>())
			{
				if (key->code == sf::Keyboard::Key::Left)
					SetValue(currentValue - step);
				else if (key->code == sf::Keyboard::Key::Right)
					SetValue(currentValue + step);
			}
		}

		Element::HandleEvent(event);
	}

	void Slider::SetValueFromMouseX(float mouseX)
	{
		const sf::Vector2f sliderPosition = GetAbsolutePosition();
		const float relativeX = mouseX - sliderPosition.x;
		const float normalized = std::clamp(relativeX / size.x, 0.0f, 1.0f);

		SetValue(minValue + normalized * (maxValue - minValue));
	}

	void Slider::RefreshLayout()
	{
		const float range = maxValue - minValue;
		const float normalized = (range > 0.0f) ? (currentValue - minValue) / range : 0.0f;

		const float handleX = normalized * size.x;

		for (Element* handle : handles)
		{
			if (handle != nullptr)
			{
				handle->anchor = { 0.0f, 0.5f };
				handle->pivot = { 0.5f, 0.5f };
				handle->offset = { handleX, 0.0f };
			}
		}

		if (activatedHandle != nullptr)
		{
			activatedHandle->anchor = { 0.0f, 0.5f };
			activatedHandle->pivot = { 0.5f, 0.5f };
			activatedHandle->offset = { handleX, 0.0f };
		}

		if (fill != nullptr)
		{
			fill->anchor = { 0.0f, 0.5f };
			fill->pivot = { 0.0f, 0.5f };
			fill->size.x = handleX;
		}
	}

	void Slider::RefreshHandleVisibility()
	{
		Element* activeHandle = isActivated && activatedHandle
			? activatedHandle
			: GetHandleForState(state);

		for (Element* handle : handles)
			if (handle != nullptr)
				handle->isVisible = (handle == activeHandle);

		if (activatedHandle != nullptr)
			activatedHandle->isVisible = (activatedHandle == activeHandle);
	}

	Element* Slider::GetHandleForState(InteractionState state) const
	{
		Element* handle = handles[StateToIndex(state)];

		return handle != nullptr ? handle : handles[StateToIndex(InteractionState::Normal)];
	}
}