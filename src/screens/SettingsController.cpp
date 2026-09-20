#include "SettingsController.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/AppDataPath.h"
#include "core/GraphicsTarget.h"
#include "core/HapticCues.h"
#include "core/Resources.h"
#include "core/Settings.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "localization/LocalizationManager.h"
#include "states/ConfirmState.h"
#include "ui/Button.h"
#include "ui/Checkbox.h"
#include "ui/Element.h"
#include "ui/Label.h"
#include "ui/Slider.h"
#include "ui/Stepper.h"

#include <SFML/Window/VideoMode.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace
{
	const std::string MenuDirectory = "data/ui/menu/";
	const std::string SettingsPath = AppDataPath::Resolve("settings.json").string();
	const std::string InputPath = AppDataPath::Resolve("input.json").string();
}

SettingsController::SettingsController(Context& context)
	: context(context)
	, settingsInterface(context.virtualScreen)
	, settingsLoader(context.resources)
{
	settingsLoader.SetButtonSounds(context.audioMixer, "ui_hover", "ui_press");
	settingsLoader.SetButtonHaptics(context.gamepadHaptics);
	settingsLoader.SetLocalization(context.localization);
	RegisterActions();
}

void SettingsController::Open(const std::string& settingsFrame)
{
	wasCloseRequested = false;
	isCapturingKey = false;
	isWaitingForKeyRelease = false;
	pendingRequest = NavRequest::None;
	activeFrame = settingsFrame;

	panelStack.clear();
	panelStack.push_back("settings");
	ShowPanel("settings");
}

void SettingsController::RegisterActions()
{
	settingsLoader.RegisterAction("menu_open_audio", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "audio"; });
	settingsLoader.RegisterAction("menu_open_graphics", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "graphics"; });
	settingsLoader.RegisterAction("menu_open_gameplay", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "gameplay"; });
	settingsLoader.RegisterAction("menu_open_controls", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "controls"; });
	settingsLoader.RegisterAction("menu_open_language", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "language"; });
	settingsLoader.RegisterAction("menu_open_keyboard", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "keyboard"; });
	settingsLoader.RegisterAction("menu_open_joystick", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "joystick"; });
	settingsLoader.RegisterAction("menu_back", [this] { pendingRequest = NavRequest::Back; });
	settingsLoader.RegisterAction("menu_save", [this] { pendingRequest = NavRequest::Save; });
	settingsLoader.RegisterAction("menu_default", [this] { ResetCurrentPanelToDefaults(); });
	settingsLoader.RegisterAction("rebind_moveleft", [this] { BeginKeyCapture(Action::MoveLeft); });
	settingsLoader.RegisterAction("rebind_moveright", [this] { BeginKeyCapture(Action::MoveRight); });
	settingsLoader.RegisterAction("rebind_jump", [this] { BeginKeyCapture(Action::Jump); });

	settingsLoader.RegisterFloatAction("set_sound_volume", [this](float value)
		{
			const int level = static_cast<int>(std::lround(value));
			const int previousLevel = context.settings.GetSoundVolume();
			context.settings.SetSoundVolume(level);
			context.audioMixer.SetSoundVolume(level / 10.0f);

			if (level != previousLevel)
				Haptics::PulseSlider(context.gamepadHaptics, level > previousLevel ? 1 : -1, level / 10.0f);

			if (auto* label = dynamic_cast<UI::Label*>(settingsInterface.FindByName("sound_value")))
				label->SetText(std::to_string(level));
		});

	settingsLoader.RegisterFloatAction("set_music_volume", [this](float value)
		{
			const int level = static_cast<int>(std::lround(value));
			const int previousLevel = context.settings.GetMusicVolume();
			context.settings.SetMusicVolume(level);
			context.audioMixer.SetMusicVolume(level / 10.0f);

			if (level != previousLevel)
				Haptics::PulseSlider(context.gamepadHaptics, level > previousLevel ? 1 : -1, level / 10.0f);

			if (auto* label = dynamic_cast<UI::Label*>(settingsInterface.FindByName("music_value")))
				label->SetText(std::to_string(level));
		});

	settingsLoader.RegisterAction("resolution_prev", [this] { StepResolution(-1); });
	settingsLoader.RegisterAction("resolution_next", [this] { StepResolution(1); });
	settingsLoader.RegisterAction("screenmode_prev", [this] { StepScreenMode(-1); });
	settingsLoader.RegisterAction("screenmode_next", [this] { StepScreenMode(1); });

	settingsLoader.RegisterBoolAction("set_vsync", [this](bool value)
		{
			context.settings.SetVsync(value);
			context.graphics.ApplyVsync(); // vsync applies immediately
		});

	settingsLoader.RegisterBoolAction("set_show_fps", [this](bool value)
		{
			context.settings.SetShowFps(value); // applies immediately, nothing to re-create
		});

	settingsLoader.RegisterBoolAction("set_vibration", [this](bool value)
		{
			context.settings.SetVibrationEnabled(value);
			context.gamepadHaptics.SetVibrationEnabled(value); // applies immediately
		});

	settingsLoader.RegisterBoolAction("set_lightbar", [this](bool value)
		{
			context.settings.SetLightbarEnabled(value);
			context.gamepadHaptics.SetLightbarEnabled(value); // applies immediately
		});

	settingsLoader.RegisterAction("language_prev", [this] { StepLanguage(-1); });
	settingsLoader.RegisterAction("language_next", [this] { StepLanguage(1); });
}

void SettingsController::ShowPanel(const std::string& panelId)
{
	std::unique_ptr<UI::Element> frame = settingsLoader.LoadFromFile(MenuDirectory + activeFrame + ".json");

	UI::Element* slot = frame->FindByName("panel_slot");
	if (slot == nullptr)
		throw std::runtime_error("SettingsController: frame.json must contain 'panel_slot'");

	// In main-menu context: the panels here already carry their own heading
	// ("Graphics", "Audio", ...), so hide the frame's title and reclaim the
	// space reserved above the panel slot for the extra row(s).
	if (activeFrame == "frame")
	{
		if (UI::Element* title = frame->FindByName("title"))
			title->isVisible = false;

		slot->offset = { 0.0f, 0.0f };
	}
	else if (panelId == "settings")
	{
		// The settings hub has no title of its own in either context (see
		// below) and its six buttons already span most of the screen, so it
		// gets the same full-height slot as the main-menu case instead of
		// pause_frame.json's default offset (which otherwise leaves room for
		// a title block this panel doesn't show).
		slot->offset = { 0.0f, 0.0f };
	}

	slot->AddChild(settingsLoader.LoadFromFile(MenuDirectory + panelId + ".json"));

	settingsInterface.SetContent(std::move(frame));

	// In pause context: show/hide the title block and container depending on whether
	// this panel has its own embedded title (audio, graphics, keyboard do; settings and
	// controls don't).
	if (activeFrame != "frame")
	{
		const bool hasOwnTitle = (panelId == "audio" || panelId == "graphics" || panelId == "gameplay"
			|| panelId == "keyboard" || panelId == "joystick" || panelId == "language");

		// The settings hub shows no title at all (not even the frame's own
		// generic "Settings" heading below) so its six-button list can use
		// the full height without one, same as in the main menu.
		const bool showsFrameTitle = !hasOwnTitle && panelId != "settings";

		if (UI::Element* block = settingsInterface.FindByName("frame_title_block"))
			block->isVisible = showsFrameTitle;

		if (UI::Element* container = settingsInterface.FindByName("frame_container"))
		{
			container->isVisible = !hasOwnTitle;

			if (panelId == "controls")
			{
				// Only 3 buttons: a shorter box to match (paired with
				// re-centering those buttons in the tighter space below).
				container->size = { 220.0f, 130.0f };
			}
			else if (panelId == "settings")
			{
				// No title above it to make room for, so it centers on the
				// whole frame instead of sitting low. Buttons are 200px wide
				// (see menu_button.json), so the box can't shrink much
				// narrower than the default without clipping them; height
				// shrinks to match the now-centered six-button list instead
				// of the taller box a title above it would have needed.
				container->size = { 220.0f, 220.0f };
				container->offset = { 0.0f, 0.0f };
			}
			else
			{
				container->size = { 220.0f, 200.0f };
			}
		}

		if (showsFrameTitle)
		{
			const std::string titleKey = (panelId == "controls") ? "settings.controls" : "settings.title";
			if (auto* label = dynamic_cast<UI::Label*>(settingsInterface.FindByName("frame_title_label")))
				label->SetText(context.localization.GetText(titleKey));

			// controls.json also carries its own heading (needed when
			// reached from the main menu, which has no frame_title_block of
			// its own) -- in pause context that would duplicate the one
			// just shown above, so hide it here.
			if (UI::Element* panelTitle = settingsInterface.FindByName("controls_panel_title"))
				panelTitle->isVisible = false;
		}

		if (panelId == "controls")
		{
			// Without its own title reserving space above them (hidden just
			// above), the three buttons re-center on the panel itself instead
			// of sitting low, leaving a gap where that title would have been.
			if (UI::Element* button = settingsInterface.FindByName("keyboard_button"))
				button->offset.y = -36.0f;
			if (UI::Element* button = settingsInterface.FindByName("joystick_button"))
				button->offset.y = 0.0f;
			if (UI::Element* button = settingsInterface.FindByName("controls_back_button"))
				button->offset.y = 36.0f;
		}
	}

	if (panelId == "audio")
		SetupAudioPanel();
	else if (panelId == "graphics")
		SetupGraphicsPanel();
	else if (panelId == "gameplay")
		SetupGameplayPanel();
	else if (panelId == "keyboard")
		SetupKeyboardPanel();
	else if (panelId == "language")
		SetupLanguagePanel();

	settingsInterface.ResetFocus();
}

void SettingsController::SetupAudioPanel()
{
	SetVolumeDisplay("sound_slider", "sound_value", context.settings.GetSoundVolume());
	SetVolumeDisplay("music_slider", "music_value", context.settings.GetMusicVolume());
}

void SettingsController::SetVolumeDisplay(const std::string& sliderName, const std::string& labelName, int value)
{
	if (auto* slider = dynamic_cast<UI::Slider*>(settingsInterface.FindByName(sliderName)))
		slider->SetValue(static_cast<float>(value));

	if (auto* label = dynamic_cast<UI::Label*>(settingsInterface.FindByName(labelName)))
		label->SetText(std::to_string(value));
}

void SettingsController::SetupGameplayPanel()
{
	if (auto* vibration = dynamic_cast<UI::Checkbox*>(settingsInterface.FindByName("vibration_checkbox")))
		vibration->SetChecked(context.settings.IsVibrationEnabled());

	if (auto* lightbar = dynamic_cast<UI::Checkbox*>(settingsInterface.FindByName("lightbar_checkbox")))
		lightbar->SetChecked(context.settings.IsLightbarEnabled());
}

void SettingsController::SetupKeyboardPanel()
{
	const Action actions[] = { Action::MoveLeft, Action::MoveRight, Action::Jump };

	for (Action action : actions)
	{
		if (auto* label = dynamic_cast<UI::Label*>(settingsInterface.FindByName(KeyLabelName(action))))
			label->SetText(Input::KeyName(context.input.GetPrimaryKey(action)));
	}
}

std::string SettingsController::KeyLabelName(Action action)
{
	switch (action)
	{
	case Action::MoveLeft:  return "moveleft_key";
	case Action::MoveRight: return "moveright_key";
	case Action::Jump:      return "jump_key";
	default:                return "";
	}
}

void SettingsController::BeginKeyCapture(Action action)
{
	isCapturingKey = true;
	captureAction = action;

	if (auto* label = dynamic_cast<UI::Label*>(settingsInterface.FindByName(KeyLabelName(action))))
		label->SetText(context.localization.GetText("controls.capturing"));
}

void SettingsController::CancelKeyCapture()
{
	isCapturingKey = false;
	isWaitingForKeyRelease = true;
	SetupKeyboardPanel();
}

void SettingsController::ApplyKeyCapture(sf::Keyboard::Key key)
{
	// Escape cancels the capture; it is reserved as the fixed pause/back key
	// and can never be bound to a game action.
	if (key == sf::Keyboard::Key::Escape)
	{
		CancelKeyCapture();
		return;
	}

	// Keep keys unique among the rebindable game actions by swapping.
	const Action editable[] = { Action::MoveLeft, Action::MoveRight, Action::Jump };
	const sf::Keyboard::Key previousKey = context.input.GetPrimaryKey(captureAction);

	for (Action other : editable)
	{
		if (other != captureAction && context.input.GetPrimaryKey(other) == key)
		{
			context.input.SetPrimaryKey(other, previousKey);
			break;
		}
	}

	context.input.SetPrimaryKey(captureAction, key);

	isCapturingKey = false;
	isWaitingForKeyRelease = true;

	SetupKeyboardPanel();
	UpdateSaveButtonTint();
}

void SettingsController::SetupGraphicsPanel()
{
	resolutions.clear();
	for (const sf::VideoMode& mode : sf::VideoMode::getFullscreenModes())
	{
		if (std::find(resolutions.begin(), resolutions.end(), mode.size) == resolutions.end())
			resolutions.push_back(mode.size);
	}

	const sf::Vector2u current(static_cast<unsigned int>(context.settings.GetResolutionWidth()),
		static_cast<unsigned int>(context.settings.GetResolutionHeight()));

	if (std::find(resolutions.begin(), resolutions.end(), current) == resolutions.end())
		resolutions.push_back(current);

	// Ascending order, so "next" (+1) means a bigger resolution: the right arrow
	// now increases the resolution as expected.
	std::sort(resolutions.begin(), resolutions.end(),
		[](const sf::Vector2u& a, const sf::Vector2u& b)
		{
			if (a.x != b.x)
				return a.x < b.x;
			return a.y < b.y;
		});

	const auto found = std::find(resolutions.begin(), resolutions.end(), current);
	resolutionIndex = static_cast<int>(std::distance(resolutions.begin(), found));

	if (!isResolutionCaptionColorKnown)
	{
		if (auto* caption = dynamic_cast<UI::Label*>(settingsInterface.FindByName("resolution_caption")))
		{
			resolutionCaptionColor = caption->GetColor();
			isResolutionCaptionColorKnown = true;
		}
	}

	UpdateResolutionLabel();
	UpdateScreenModeLabel();
	UpdateResolutionRowEnabled();

	if (auto* vsync = dynamic_cast<UI::Checkbox*>(settingsInterface.FindByName("vsync_checkbox")))
		vsync->SetChecked(context.settings.IsVsyncEnabled());

	if (auto* showFps = dynamic_cast<UI::Checkbox*>(settingsInterface.FindByName("show_fps_checkbox")))
		showFps->SetChecked(context.settings.IsShowFpsEnabled());
}

void SettingsController::StepResolution(int direction)
{
	if (context.settings.GetScreenMode() == ScreenMode::Borderless)
		return; // borderless uses the desktop resolution
	if (resolutions.empty())
		return;

	resolutionIndex = std::clamp(resolutionIndex + direction, 0, static_cast<int>(resolutions.size()) - 1);

	const sf::Vector2u resolution = resolutions[resolutionIndex];
	context.settings.SetResolution(static_cast<int>(resolution.x), static_cast<int>(resolution.y));

	UpdateResolutionLabel();
}

void SettingsController::StepScreenMode(int direction)
{
	const ScreenMode order[3] = { ScreenMode::Fullscreen, ScreenMode::Borderless, ScreenMode::Window };

	int current = 0;
	for (int i = 0; i < 3; i++)
		if (order[i] == context.settings.GetScreenMode())
			current = i;

	current = (current + direction + 3) % 3;
	context.settings.SetScreenMode(order[current]);

	UpdateScreenModeLabel();
	UpdateResolutionRowEnabled();
}

void SettingsController::UpdateResolutionLabel()
{
	if (auto* label = dynamic_cast<UI::Label*>(settingsInterface.FindByName("resolution_value")))
		label->SetText(std::to_string(context.settings.GetResolutionWidth()) + " x "
			+ std::to_string(context.settings.GetResolutionHeight()));
}

void SettingsController::UpdateScreenModeLabel()
{
	std::string key = "graphics.mode_borderless";
	switch (context.settings.GetScreenMode())
	{
	case ScreenMode::Fullscreen: key = "graphics.mode_fullscreen"; break;
	case ScreenMode::Borderless: key = "graphics.mode_borderless"; break;
	case ScreenMode::Window:     key = "graphics.mode_window"; break;
	}

	const std::string text = context.localization.GetText(key);

	if (auto* label = dynamic_cast<UI::Label*>(settingsInterface.FindByName("screenmode_value")))
		label->SetText(text);
}

void SettingsController::UpdateResolutionRowEnabled()
{
	const bool enabled = context.settings.GetScreenMode() != ScreenMode::Borderless;

	// The stepper grays its own arrows and value text (disabled color comes from
	// its data). Here we only need to mirror that on the separate caption label.
	auto* stepper = dynamic_cast<UI::Stepper*>(settingsInterface.FindByName("resolution_stepper"));
	if (stepper != nullptr)
		stepper->SetEnabled(enabled);

	if (auto* caption = dynamic_cast<UI::Label*>(settingsInterface.FindByName("resolution_caption")))
	{
		const sf::Color disabledColor = (stepper != nullptr) ? stepper->GetDisabledColor() : caption->GetColor();
		caption->SetColor(enabled ? resolutionCaptionColor : disabledColor);
	}
}

void SettingsController::SetupLanguagePanel()
{
	UpdateLanguageLabel();
}

void SettingsController::StepLanguage(int direction)
{
	const Language order[5] = { Language::English, Language::German, Language::Spanish, Language::Russian, Language::Ukrainian };

	int current = 0;
	for (int i = 0; i < 5; i++)
		if (order[i] == context.settings.GetLanguage())
			current = i;

	current = (current + direction + 5) % 5;
	context.settings.SetLanguage(order[current]);

	UpdateLanguageLabel();
}

void SettingsController::UpdateLanguageLabel()
{
	if (auto* label = dynamic_cast<UI::Label*>(settingsInterface.FindByName("language_value")))
		label->SetText(context.localization.GetLanguageDisplayName(context.settings.GetLanguage()));
}

bool SettingsController::IsSettingsPanel(const std::string& panelId) const
{
	return panelId == "audio" || panelId == "graphics" || panelId == "gameplay"
		|| panelId == "keyboard" || panelId == "language";
}

void SettingsController::ResetCurrentPanelToDefaults()
{
	if (panelStack.empty())
		return;

	const std::string& panel = panelStack.back();

	if (panel == "audio")
	{
		context.settings.ResetAudioToDefaults();
		context.audioMixer.SetSoundVolume(context.settings.GetSoundVolume() / 10.0f);
		context.audioMixer.SetMusicVolume(context.settings.GetMusicVolume() / 10.0f);
		SetupAudioPanel();
	}
	else if (panel == "graphics")
	{
		context.settings.ResetGraphicsToDefaults();
		context.graphics.ApplyVsync(); // vsync applies live; resolution/mode wait for Save
		SetupGraphicsPanel();
	}
	else if (panel == "gameplay")
	{
		context.settings.ResetGameplayToDefaults();
		context.gamepadHaptics.SetVibrationEnabled(context.settings.IsVibrationEnabled());
		context.gamepadHaptics.SetLightbarEnabled(context.settings.IsLightbarEnabled());
		SetupGameplayPanel();
	}
	else if (panel == "keyboard")
	{
		context.input.ResetToDefaults();
		SetupKeyboardPanel();
	}
	else if (panel == "language")
	{
		context.settings.SetLanguage(Language::English);
		UpdateLanguageLabel();
	}

	UpdateSaveButtonTint();
}

void SettingsController::GoBackPanel()
{
	if (panelStack.size() > 1)
	{
		panelStack.pop_back();
		ShowPanel(panelStack.back());
	}
}

void SettingsController::UpdateSaveButtonTint()
{
	auto* save = dynamic_cast<UI::Button*>(settingsInterface.FindByName("save_button"));
	if (save == nullptr)
		return;

	const sf::Color clean(120, 200, 120, 255); // green: nothing to save
	const sf::Color dirty(230, 150, 80, 255);  // orange: unsaved changes
	save->SetBackgroundTint(PanelIsDirty(panelStack.back()) ? dirty : clean);
}

void SettingsController::OpenUnsavedChangesDialog()
{
	context.stateMachine.Push(std::make_unique<ConfirmState>(context,
		context.localization.GetText("dialog.warning_title"),
		context.localization.GetText("dialog.unsaved_changes_message"),
		[this] { SaveAndGoBack(); },
		[this] { RevertAndGoBack(); }));
}

void SettingsController::SaveAndGoBack()
{
	SavePanel(panelStack.back());
	GoBackPanel();
}

void SettingsController::RevertAndGoBack()
{
	RevertPanel(panelStack.back());
	GoBackPanel();
}

bool SettingsController::PanelIsDirty(const std::string& panel) const
{
	if (panel == "keyboard")
		return context.input.IsDirty();
	if (panel == "audio" || panel == "graphics" || panel == "gameplay" || panel == "language")
		return context.settings.IsDirty();
	return false;
}

void SettingsController::SavePanel(const std::string& panel)
{
	// If the write fails (disk full, file locked, ...), Save()/SaveConfig()
	// leave the in-memory state dirty rather than pretending it succeeded,
	// so the existing "unsaved changes" prompt keeps firing on the next
	// attempt to leave the panel instead of silently losing the changes.
	if (panel == "keyboard")
	{
		context.input.SaveConfig(InputPath);
	}
	else if (panel == "audio" || panel == "graphics" || panel == "gameplay" || panel == "language")
	{
		context.settings.Save(SettingsPath);
		context.graphics.ApplyGraphics();

		// The chosen language only takes effect on Save (like resolution and
		// screen mode), not while the stepper is still being cycled.
		const bool languageChanged = context.localization.GetLanguage() != context.settings.GetLanguage();
		context.localization.SetLanguage(context.settings.GetLanguage());

		// A LocalizationManager revision bump does not retranslate anything by
		// itself -- every Label/TextBox already holds its resolved sf::Text, not
		// its textKey. Reloading the panel from JSON (same as opening it fresh)
		// re-resolves every textKey against the new catalog, so the panel the
		// player is looking at -- including its own title/buttons, not just the
		// stepper value -- updates immediately instead of on the next visit.
		if (languageChanged)
			ShowPanel(panelStack.back());
	}
}

void SettingsController::RevertPanel(const std::string& panel)
{
	if (panel == "keyboard")
	{
		context.input.Revert();
	}
	else if (panel == "audio" || panel == "graphics" || panel == "gameplay" || panel == "language")
	{
		context.settings.Revert();
		context.audioMixer.SetSoundVolume(context.settings.GetSoundVolume() / 10.0f);
		context.audioMixer.SetMusicVolume(context.settings.GetMusicVolume() / 10.0f);
		context.graphics.ApplyVsync();
		context.gamepadHaptics.SetVibrationEnabled(context.settings.IsVibrationEnabled());
		context.gamepadHaptics.SetLightbarEnabled(context.settings.IsLightbarEnabled());
	}
}

void SettingsController::ApplyPendingNavigation()
{
	switch (pendingRequest)
	{
	case NavRequest::OpenPanel:
		panelStack.push_back(pendingPanelId);
		ShowPanel(pendingPanelId);
		break;

	case NavRequest::Back:
		if (panelStack.size() > 1)
		{
			if (IsSettingsPanel(panelStack.back()) && PanelIsDirty(panelStack.back()))
				OpenUnsavedChangesDialog();
			else
				GoBackPanel();
		}
		else
		{
			wasCloseRequested = true; // backed out of the settings root: hand control back
		}
		break;

	case NavRequest::Save:
		SavePanel(panelStack.back());
		break;

	case NavRequest::None:
		break;
	}

	pendingRequest = NavRequest::None;
}

void SettingsController::HandleEvent(const sf::Event& event)
{
	if (isCapturingKey)
	{
		if (const auto* key = event.getIf<sf::Event::KeyPressed>())
			ApplyKeyCapture(key->code);

		return; // swallow every event until a key is pressed
	}

	settingsInterface.HandleEvent(event);
}

void SettingsController::Update(float deltaTime)
{
	settingsInterface.Update(deltaTime);

	Input& input = context.input;

	// A gamepad has no keyboard event to swallow in HandleEvent, so give it a
	// way out of key capture here: its "back" button cancels the rebind, same
	// as pressing Escape.
	if (isCapturingKey && input.WasPressed(Action::MenuBack))
	{
		CancelKeyCapture();
		Haptics::PulsePress(context.gamepadHaptics);
		return;
	}

	if (isWaitingForKeyRelease && !isCapturingKey)
	{
		const bool anyMenuKeyDown = input.IsDown(Action::MenuUp) || input.IsDown(Action::MenuDown)
			|| input.IsDown(Action::MenuLeft) || input.IsDown(Action::MenuRight)
			|| input.IsDown(Action::MenuConfirm) || input.IsDown(Action::MenuBack);
		if (!anyMenuKeyDown)
			isWaitingForKeyRelease = false;
	}

	// While capturing a key (or until the keys from a finished capture are
	// released) menu navigation is suppressed, so the rebound key does not also
	// trigger a navigation action.
	if (!isCapturingKey && !isWaitingForKeyRelease)
	{
		if (input.WasPressed(Action::MenuBack))
		{
			pendingRequest = NavRequest::Back;
			Haptics::PulsePress(context.gamepadHaptics);
		}
		else if (input.WasPressed(Action::MenuDown))
		{
			settingsInterface.NavigateDown();
		}
		else if (input.WasPressed(Action::MenuUp))
		{
			settingsInterface.NavigateUp();
		}
		else if (input.WasPressed(Action::MenuLeft))
		{
			settingsInterface.NavigateLeft();
		}
		else if (input.WasPressed(Action::MenuRight))
		{
			settingsInterface.NavigateRight();
		}

		if (input.WasPressed(Action::MenuConfirm))
			settingsInterface.Confirm(true);
		else if (input.WasReleased(Action::MenuConfirm))
			settingsInterface.Confirm(false);
	}

	UpdateSaveButtonTint();
	ApplyPendingNavigation();
}

void SettingsController::Render(sf::RenderTarget& target)
{
	context.virtualScreen.SetCameraCenter(VirtualScreen::Width / 2.0f, VirtualScreen::Height / 2.0f);
	settingsInterface.Draw(target);
}