#pragma once

#include "ui/InteractiveElement.h"

#include <array>
#include <functional>
#include <memory>

namespace UI
{
	class Slider : public InteractiveElement
	{
	public:
		void SetTrack(std::unique_ptr<Element> element);
		void SetFill(std::unique_ptr<Element> element);
		void SetHandle(InteractionState state, std::unique_ptr<Element> element);
		void SetActivatedHandle(std::unique_ptr<Element> element);

		void SetRange(float minValue, float maxValue);
		void SetStep(float step);
		void SetValue(float value);
		float GetValue() const { return currentValue; }

		void SetOnValueChanged(std::function<void(float)> callback);

		bool RequiresActivation() const override { return true; }

		void OnDragStart(sf::Vector2f mousePosition) override;
		void OnDragMove(sf::Vector2f mousePosition) override;
		void OnNavigate(int direction) override;

		void HandleEvent(const sf::Event& event) override;

	protected:
		void OnStateChanged() override;
		void OnActivated() override;
		void OnDeactivated() override;

	private:
		void RefreshLayout();
		void RefreshHandleVisibility();
		void SetValueFromMouseX(float mouseX);

		Element* GetHandleForState(InteractionState state) const;

		Element* track = nullptr;
		Element* fill = nullptr;
		std::array<Element*, INTERACTION_STATE_COUNT> handles = { nullptr, nullptr, nullptr };
		Element* activatedHandle = nullptr;

		float minValue = 0.0f;
		float maxValue = 1.0f;
		float currentValue = 0.0f;
		float step = 0.1f;

		std::function<void(float)> onValueChanged;
	};
}