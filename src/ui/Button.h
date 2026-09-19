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
		void SetBackgroundTint(sf::Color color);

	protected:
		void OnStateChanged() override;

	private:
		void RefreshVisibility();

		Element* GetVariant(const std::array<Element*, InteractionStateCount>& variants, InteractionState state) const;

		std::array<Element*, InteractionStateCount> backgrounds = { nullptr, nullptr, nullptr };
		std::array<Element*, InteractionStateCount> foregrounds = { nullptr, nullptr, nullptr };
		std::array<std::optional<sf::Color>, InteractionStateCount> foregroundColors = { std::nullopt, std::nullopt, std::nullopt };
	};
}