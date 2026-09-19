#pragma once

#include "ui/InteractiveElement.h"

#include <SFML/Graphics/Color.hpp>

#include <array>
#include <functional>
#include <memory>

namespace UI
{
	// A "left value right" control. It is a value control, so while focused the
	// left/right keys step it directly. Each arrow has a normal image (tinted
	// brighter on hover) and a pressed image shown while that arrow is clicked.
	class Stepper : public InteractiveElement
	{
	public:
		void SetLeftArrowNormal(std::unique_ptr<Element> element);
		void SetLeftArrowPressed(std::unique_ptr<Element> element);
		void SetRightArrowNormal(std::unique_ptr<Element> element);
		void SetRightArrowPressed(std::unique_ptr<Element> element);
		void SetValueLabel(std::unique_ptr<Element> element);

		void SetArrowColor(InteractionState state, sf::Color color);
		void SetValueColor(sf::Color color);

		void SetOnStepLeft(std::function<void()> callback);
		void SetOnStepRight(std::function<void()> callback);

		bool IsValueControl() const override { return true; }

		void OnDragStart(sf::Vector2f mousePosition) override;
		void OnDragEnd() override;
		void OnNavigate(int direction) override;

		void Update(float deltaTime) override;

	protected:
		void OnStateChanged() override;
		void OnEnabledChanged() override;

	private:
		enum class PressedArrow
		{
			None,
			Left,
			Right
		};

		void Refresh();
		sf::Color CurrentArrowTint() const;

		Element* leftArrowNormal = nullptr;
		Element* leftArrowPressed = nullptr;
		Element* rightArrowNormal = nullptr;
		Element* rightArrowPressed = nullptr;
		Element* valueLabel = nullptr;

		// Only Normal and Highlighted tints are used; Pressed is shown by swapping
		// to the pressed image instead of tinting.
		std::array<sf::Color, InteractionStateCount> arrowColors =
		{
			sf::Color::White,
			sf::Color::White,
			sf::Color::White
		};
		sf::Color valueColor = sf::Color::White;

		PressedArrow pressedArrow = PressedArrow::None;
		float pressedTimer = 0.0f; // brief flash of the pressed image on keyboard step

		std::function<void()> onStepLeft;
		std::function<void()> onStepRight;
	};
}