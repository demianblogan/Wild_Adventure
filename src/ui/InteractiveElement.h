#pragma once

#include "ui/Element.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <functional>

namespace UI
{
	enum class InteractionState
	{
		Normal,
		Highlighted,
		Pressed
	};

	constexpr std::size_t InteractionStateCount = 3;

	// Maps a state to its slot in a per-state array (backgrounds/handles/colors
	// indexed by InteractionState in Button, Checkbox, Slider and Stepper).
	int StateToIndex(InteractionState state);

	class InteractiveElement : public Element
	{
	public:
		bool IsInteractive() const override { return true; }

		void SetHighlighted(bool highlighted, bool playSound = true);
		void Press();
		void Release();
		void ClearPressed(); // drop the pressed visual without firing the action

		void Activate();
		void Deactivate();

		// True for controls whose value is changed with left/right (slider, stepper).
		// Such a control is auto-activated while focused, so left/right adjust it
		// directly instead of moving between sibling elements.
		virtual bool IsValueControl() const { return false; }
		
		bool IsActivated() const { return isActivated; }

		void SetEnabled(bool enabled);
		bool IsEnabled() const { return isEnabled; }

		void SetDisabledColor(sf::Color color) { disabledColor = color; }
		sf::Color GetDisabledColor() const { return disabledColor; }

		void SetOnPressed(std::function<void()> callback);
		void SetOnHighlighted(std::function<void()> callback);

		InteractionState GetState() const { return state; }

		// When true (the default), Root draws the highlighted element once
		// more into the glow layer for a soft bloom. Text-only controls that
		// already pulse their own alpha/size (e.g. the language picker list)
		// turn this off so the glow doesn't wash the text out mid-pulse.
		bool bloomsWhenHighlighted = true;

		virtual void OnDragStart(sf::Vector2f) {}
		virtual void OnDragMove(sf::Vector2f) {}
		virtual void OnDragEnd() {}
		virtual void OnNavigate(int) {}

	protected:
		virtual void OnStateChanged() {}
		virtual void OnActivated() {}
		virtual void OnDeactivated() {}
		virtual void OnEnabledChanged() {}

		InteractionState state = InteractionState::Normal;
		bool isActivated = false;
		bool isEnabled = true;

		sf::Color disabledColor = sf::Color(120, 120, 120, 255);

		std::function<void()> onPressed;
		std::function<void()> onHighlighted;
	};
}