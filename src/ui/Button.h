#pragma once

#include "ui/InteractiveElement.h"

#include <array>
#include <memory>

namespace UI
{
	class Button : public InteractiveElement
	{
	public:
		Button();

		void SetBackground(InteractionState state, std::unique_ptr<Element> element);
		void SetForeground(InteractionState state, std::unique_ptr<Element> element);

	protected:
		void OnStateChanged() override;

	private:
		void RefreshVisibility();

		Element* GetVariant(const std::array<Element*, 3>& variants, InteractionState state) const;

		std::array<Element*, 3> backgrounds = { nullptr, nullptr, nullptr };
		std::array<Element*, 3> foregrounds = { nullptr, nullptr, nullptr };
	};
}