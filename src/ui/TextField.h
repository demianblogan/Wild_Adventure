#pragma once

#include "ui/InteractiveElement.h"

#include <array>
#include <functional>
#include <memory>
#include <string>

struct Resources;

namespace UI
{
	class Label;
	class Image;

	class TextField : public InteractiveElement
	{
	public:
		TextField(Resources& resources, const std::string& fontName);

		void SetBackground(InteractionState state, std::unique_ptr<Element> element);

		void SetText(const std::string& text);
		const std::string& GetText() const { return text; }

		void SetFilter(std::function<bool(char32_t)> filter);
		void SetOnTextChanged(std::function<void(const std::string&)> callback);

		bool RequiresActivation() const override { return true; }

		void Update(float deltaTime) override;
		void HandleEvent(const sf::Event& event) override;

	protected:
		void OnStateChanged() override;
		void OnActivated() override;
		void OnDeactivated() override;

	private:
		void RefreshBackgroundVisibility();
		void RefreshTextDisplay();
		void RefreshCursorPosition();
		void NotifyChanged();

		void InsertCharacter(char32_t character);
		void DeleteBefore();
		void DeleteAfter();
		void MoveCursorLeft();
		void MoveCursorRight();

		Element* GetBackgroundForState(InteractionState state) const;

		Resources& resources;
		std::string fontName;

		Label* textLabel = nullptr;
		Image* cursor = nullptr;
		std::array<Element*, 3> backgrounds = { nullptr, nullptr, nullptr };

		std::string text;
		std::size_t cursorPosition = 0;

		std::function<bool(char32_t)> filter;
		std::function<void(const std::string&)> onTextChanged;

		float blinkTimer = 0.0f;
		float blinkPeriod = 0.5f;
		bool cursorBlinkVisible = true;
	};
}