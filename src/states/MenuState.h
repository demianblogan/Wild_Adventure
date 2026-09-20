#pragma once

#include "core/Input.h"
#include "core/State.h"
#include "graphics/Transition.h"
#include "screens/CharacterSelectController.h"
#include "screens/MenuBackdrop.h"
#include "screens/SelectLevelController.h"
#include "screens/SettingsController.h"
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
	enum class NavRequest { None, OpenPanel, Back, StartGame, ContinueGame, Exit };

	void RegisterActions();
	void ShowPanel(const std::string& panelId);
	void SetupPlayPanel();
	void GoBackPanel();
	void OpenQuitDialog();
	void ApplyPendingNavigation();
	void OpenCharacterSelect(int levelNumber);

	MenuBackdrop backdrop;

	UI::Root userInterface;
	UI::DataLoader interfaceLoader;

	Transition transition;

	SettingsController settings;
	bool isInSettings = false;
	int lastLocalizationRevision = 0; // detects a language change made while isInSettings was true

	SelectLevelController selectLevel;
	bool isInSelectLevel = false;

	CharacterSelectController characterSelect;
	bool isInCharacterSelect = false;

	std::vector<std::string> panelStack;

	NavRequest pendingRequest = NavRequest::None;
	std::string pendingPanelId;
};