#pragma once

#include "ui/Element.h"

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

	constexpr std::size_t INTERACTION_STATE_COUNT = 3;

	class InteractiveElement : public Element
	{
	public:
		bool IsInteractive() const override { return true; }

		void SetHighlighted(bool highlighted, bool playSound = true);
		void Press();
		void Release();

		void Activate();
		void Deactivate();

		virtual bool RequiresActivation() const { return false; }
		bool IsActivated() const { return isActivated; }

		void SetOnPressed(std::function<void()> callback);
		void SetOnHighlighted(std::function<void()> callback);

		InteractionState GetState() const { return state; }

		virtual void OnDragStart(sf::Vector2f mousePosition) {}
		virtual void OnDragMove(sf::Vector2f mousePosition) {}
		virtual void OnDragEnd() {}
		virtual void OnNavigate(int direction) {}

	protected:
		virtual void OnStateChanged() {}
		virtual void OnActivated() {}
		virtual void OnDeactivated() {}


		InteractionState state = InteractionState::Normal;
		bool isActivated = false;

		std::function<void()> onPressed;
		std::function<void()> onHighlighted;
	};
}