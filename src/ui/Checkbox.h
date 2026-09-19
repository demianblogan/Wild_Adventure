#pragma once

#include "ui/InteractiveElement.h"

#include <SFML/Graphics/Color.hpp>

#include <array>
#include <functional>
#include <memory>
#include <optional>

namespace UI
{
	class Checkbox : public InteractiveElement
	{
	public:
		Checkbox();

		void SetBackground(InteractionState state, std::unique_ptr<Element> element);
		void SetCheckedView(std::unique_ptr<Element> element);
		void SetUncheckedView(std::unique_ptr<Element> element);

		// Optional tint applied to the visible view per interaction state, so the
		// checkbox shows focus/hover (e.g. dimmer when idle, brighter when focused).
		void SetViewColor(InteractionState state, sf::Color color);

		void SetChecked(bool checked);
		bool IsChecked() const { return isChecked; }

		void SetOnCheckedChanged(std::function<void(bool)> callback);

	protected:
		void OnStateChanged() override;

	private:
		void HandleRelease();
		void RefreshVisibility();

		Element* GetBackgroundForState(InteractionState state) const;

		std::array<Element*, InteractionStateCount> backgrounds = { nullptr, nullptr, nullptr };
		Element* checkedView = nullptr;
		Element* uncheckedView = nullptr;

		std::array<std::optional<sf::Color>, InteractionStateCount> viewColors = {};

		bool isChecked = false;
		std::function<void(bool)> onCheckedChanged;
	};
}