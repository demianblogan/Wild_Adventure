#pragma once

#include "core/State.h"
#include "states/SettingsController.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

#include <string>

class PauseState : public State
{
public:
	PauseState(Context& context, std::string levelPath, int levelNumber);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	enum class NavRequest { None, Continue, Restart, Options, QuitToMenu };

	void RegisterActions();
	void ApplyPendingNavigation();

	UI::Root pauseInterface;
	UI::DataLoader pauseLoader;
	SettingsController settings;

	bool inSettings = false;
	NavRequest pendingRequest = NavRequest::None;

	std::string levelPath;
	int levelNumber;
};
