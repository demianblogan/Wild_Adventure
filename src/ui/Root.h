#pragma once

#include "ui/Element.h"

#include <SFML/System/Vector2.hpp>

#include <memory>
#include <vector>

namespace sf
{
	class RenderTarget;
	class Event;
}

class VirtualScreen;

namespace UI
{
	class InteractiveElement;

	enum class InputMode
	{
		Cursor,
		Selection
	};

	class Root
	{
	public:
		Root(VirtualScreen& virtualScreen);

		Element& SetContent(std::unique_ptr<Element> content);

		// Re-scans the current content for navigable elements and resets
		// focus. Call this after changing an element's isVisible outside of
		// SetContent (e.g. hiding a locked button once its unlock state is
		// known) -- CollectInteractives() otherwise only ever runs once, at
		// SetContent time, so a later visibility change wouldn't otherwise
		// be reflected in what Up/Down/mouse-hover can reach.
		void RefreshInteractives();

		Element* FindByName(const std::string& name);

		void HandleEvent(const sf::Event& event);
		void Update(float deltaTime);
		void Draw(sf::RenderTarget& target) const;

		void NavigateUp();
		void NavigateDown();
		void NavigateLeft();
		void NavigateRight();
		void Confirm(bool pressed);
		void ResetFocus();

	private:
		void CollectInteractives();
		void CollectInteractivesFrom(Element& element, std::vector<InteractiveElement*>* currentRow);
		void CollectGlowingFrom(Element& element);

		void HandleMouseMove();
		void HandleMousePress();
		void HandleMouseRelease();

		void MoveRow(int direction);
		void MoveColumn(int direction);
		void HandleConfirm(bool pressed);

		void SetFocus(int row, int column, bool playSound = true);
		void ClearFocus();
		void DeactivateFocused();

		InteractiveElement* CurrentElement() const;
		int FirstEnabledColumn(int row, int preferredColumn) const;

		VirtualScreen& virtualScreen;
		std::unique_ptr<Element> content;
		std::vector<std::vector<InteractiveElement*>> rows;
		std::vector<Element*> glowingElements; // always-glowing decorations (e.g. the title)

		InputMode activeMode = InputMode::Cursor;
		int focusRow = -1;
		int focusColumn = -1;
		int desiredColumn = 0; // remembered column when moving between rows
		InteractiveElement* draggedElement = nullptr;
		InteractiveElement* activatedElement = nullptr;

		bool isConfirmHeld = false; // true while the confirm key is held down
	};
}