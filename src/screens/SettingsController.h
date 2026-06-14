#pragma once

#include "core/Input.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <string>
#include <vector>

class Context;

namespace sf
{
	class Event;
	class RenderTarget;
}

// Self-contained settings UI (graphics / audio / controls), shared by the main
// menu and the pause menu. The owner calls Open(), then forwards events / update /
// render while the settings are shown; when the user backs out of the root panel
// WantsClose() turns true and the owner hides the settings again.
class SettingsController
{
public:
	SettingsController(Context& context);

	void Open(const std::string& settingsFrame = "frame");
	bool WantsClose() const { return wantsClose; }

	void HandleEvent(const sf::Event& event);
	void Update(float deltaTime);
	void Render(sf::RenderTarget& target);

private:
	enum class NavRequest { None, OpenPanel, Back, Save };

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

	Context& context;

	UI::Root settingsInterface;
	UI::DataLoader settingsLoader;

	std::vector<std::string> panelStack;
	std::string activeFrame = "frame";
	bool wantsClose = false;

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