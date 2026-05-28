#pragma once

#include "ui/Element.h"

#include <functional>

namespace UI
{
	enum class InteractionState
	{
		Normal,
		Highlighted,
		Pressed
	};

	class InteractiveElement : public Element
	{
	public:
		bool IsInteractive() const override { return true; }

		void SetHighlighted(bool highlighted);
		void Press();
		void Release();

		void SetOnPressed(std::function<void()> callback);

		InteractionState GetState() const { return state; }

	protected:
		virtual void OnStateChanged() {}

		InteractionState state = InteractionState::Normal;
		std::function<void()> onPressed;
	};
}