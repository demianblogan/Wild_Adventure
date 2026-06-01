#pragma once

#include "ui/InteractiveElement.h"

#include <array>
#include <functional>
#include <memory>

namespace UI
{
	class Checkbox : public InteractiveElement
	{
	public:
		Checkbox();

		void SetBackground(InteractionState state, std::unique_ptr<Element> element);
		void SetCheckedView(std::unique_ptr<Element> element);
		void SetUncheckedView(std::unique_ptr<Element> element);

		void SetChecked(bool checked);
		bool IsChecked() const { return isChecked; }

		void SetOnCheckedChanged(std::function<void(bool)> callback);

	protected:
		void OnStateChanged() override;

	private:
		void HandleRelease();
		void RefreshVisibility();

		Element* GetBackgroundForState(InteractionState state) const;

		std::array<Element*, INTERACTION_STATE_COUNT> backgrounds = { nullptr, nullptr, nullptr };
		Element* checkedView = nullptr;
		Element* uncheckedView = nullptr;

		bool isChecked = false;
		std::function<void(bool)> onCheckedChanged;
	};
}