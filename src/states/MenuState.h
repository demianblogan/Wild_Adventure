#pragma once

#include "core/State.h"
#include "graphics/Transition.h"
#include "states/MenuBackdrop.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

#include <string>
#include <vector>

class MenuState : public State
{
public:
	MenuState(Context& context);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	enum class NavRequest { None, OpenPanel, Back, StartGame, Exit };

	void RegisterActions();
	void ShowPanel(const std::string& panelId);
	void ApplyPendingNavigation();

	MenuBackdrop backdrop;

	UI::Root userInterface;
	UI::DataLoader interfaceLoader;

	Transition transition;

	std::vector<std::string> panelStack;

	NavRequest pendingRequest = NavRequest::None;
	std::string pendingPanelId;
};