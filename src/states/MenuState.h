#pragma once

#include "core/Input.h"
#include "core/State.h"
#include "graphics/Transition.h"
#include "states/MenuBackdrop.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Vector2.hpp>

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
	enum class NavRequest { None, OpenPanel, Back, StartGame, Save, Exit };

	void RegisterActions();
	void ShowPanel(const std::string& panelId);
	void ApplyPendingNavigation();

	void SetupAudioPanel();
	void SetVolumeDisplay(const std::string& sliderName, const std::string& labelName, int value);

	void SetupKeyboardPanel();
	void BeginKeyCapture(Action action);
	void ApplyKeyCapture(sf::Keyboard::Key key);
	static std::string KeyLabelName(Action action);

	bool PanelIsDirty(const std::string& panel) const;
	void SavePanel(const std::string& panel);
	void RevertPanel(const std::string& panel);

	void GoBackPanel();
	void UpdateSaveButtonTint();
	void ResetCurrentPanelToDefaults();
	void OpenUnsavedChangesDialog();
	void SaveAndGoBack();
	void RevertAndGoBack();
	bool IsSettingsPanel(const std::string& panelId) const;

	void SetupGraphicsPanel();
	void UpdateResolutionLabel();
	void UpdateScreenModeLabel();
	void UpdateResolutionRowEnabled();
	void StepResolution(int direction);
	void StepScreenMode(int direction);

	MenuBackdrop backdrop;

	UI::Root userInterface;
	UI::DataLoader interfaceLoader;

	Transition transition;

	std::vector<std::string> panelStack;

	NavRequest pendingRequest = NavRequest::None;
	std::string pendingPanelId;

	std::vector<sf::Vector2u> resolutions;
	int resolutionIndex = 0;

	sf::Color resolutionCaptionColor; // caption color while the resolution row is enabled
	bool resolutionCaptionColorKnown = false;

	bool capturingKey = false;
	Action captureAction = Action::MoveLeft;
	bool waitForKeyRelease = false; // suppress nav until keys from a finished capture are released
};