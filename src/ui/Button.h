#pragma once

#include "ui/InteractiveElement.h"

#include <SFML/Graphics/Color.hpp>

#include <array>
#include <memory>
#include <optional>

namespace UI
{
	class Button : public InteractiveElement
	{
	public:
		void SetBackground(InteractionState state, std::unique_ptr<Element> element);
		void SetForeground(InteractionState state, std::unique_ptr<Element> element);
		void SetForegroundColor(InteractionState state, sf::Color color);

	protected:
		void OnStateChanged() override;

	private:
		void RefreshVisibility();

		Element* GetVariant(const std::array<Element*, INTERACTION_STATE_COUNT>& variants, InteractionState state) const;

		std::array<Element*, INTERACTION_STATE_COUNT> backgrounds = { nullptr, nullptr, nullptr };
		std::array<Element*, INTERACTION_STATE_COUNT> foregrounds = { nullptr, nullptr, nullptr };
		std::array<std::optional<sf::Color>, INTERACTION_STATE_COUNT> foregroundColors = { std::nullopt, std::nullopt, std::nullopt };
	};
}