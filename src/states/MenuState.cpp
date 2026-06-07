#include "MenuState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/Settings.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "states/ConfirmState.h"
#include "states/GameState.h"
#include "ui/Button.h"
#include "ui/Checkbox.h"
#include "ui/Element.h"
#include "ui/Label.h"
#include "ui/Slider.h"
#include "core/GraphicsTarget.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

namespace
{
	const std::string MENU_DIRECTORY = "data/ui/menu/";
	const std::string SETTINGS_PATH = "data/settings.json";
}

MenuState::MenuState(Context& context)
	: State(context)
	, backdrop(context)
	, userInterface(context.virtualScreen)
	, interfaceLoader(context.resources)
{
	Resources& resources = context.resources;

	if (!resources.fonts.Has("main"))
	{
		resources.fonts.Load("main", "assets/fonts/main.ttf");
		resources.fonts.Get("main").setSmooth(false);
	}

	interfaceLoader.SetButtonSounds(context.audioMixer, "ui_hover", "ui_press");

	RegisterActions();

	panelStack.push_back("main");
	ShowPanel("main");

	transition.StartReveal();
}

void MenuState::RegisterActions()
{
	interfaceLoader.RegisterAction("menu_open_play", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "play"; });
	interfaceLoader.RegisterAction("menu_open_single", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "single"; });
	interfaceLoader.RegisterAction("menu_open_settings", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "settings"; });
	interfaceLoader.RegisterAction("menu_open_audio", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "audio"; });
	interfaceLoader.RegisterAction("menu_open_graphics", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "graphics"; });
	interfaceLoader.RegisterAction("menu_open_author", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "author"; });
	interfaceLoader.RegisterAction("menu_back", [this] { pendingRequest = NavRequest::Back; });
	interfaceLoader.RegisterAction("menu_save", [this] { pendingRequest = NavRequest::Save; });
	interfaceLoader.RegisterAction("menu_exit", [this] { pendingRequest = NavRequest::Exit; });
	interfaceLoader.RegisterAction("menu_start_game", [this] { pendingRequest = NavRequest::StartGame; });

	interfaceLoader.RegisterFloatAction("set_sound_volume", [this](float value)
		{
			const int level = static_cast<int>(std::lround(value));
			context.settings.SetSoundVolume(level);
			context.audioMixer.SetSoundVolume(level / 10.0f);

			if (auto* label = dynamic_cast<UI::Label*>(userInterface.FindByName("sound_value")))
				label->SetText(std::to_string(level));
		});

	interfaceLoader.RegisterFloatAction("set_music_volume", [this](float value)
		{
			const int level = static_cast<int>(std::lround(value));
			context.settings.SetMusicVolume(level);
			context.audioMixer.SetMusicVolume(level / 10.0f);

			if (auto* label = dynamic_cast<UI::Label*>(userInterface.FindByName("music_value")))
				label->SetText(std::to_string(level));
		});

	interfaceLoader.RegisterAction("resolution_prev", [this] { StepResolution(-1); });
	interfaceLoader.RegisterAction("resolution_next", [this] { StepResolution(1); });
	interfaceLoader.RegisterAction("screenmode_prev", [this] { StepScreenMode(-1); });
	interfaceLoader.RegisterAction("screenmode_next", [this] { StepScreenMode(1); });

	interfaceLoader.RegisterBoolAction("set_vsync", [this](bool value)
		{
			context.settings.SetVsync(value);
			context.graphics.ApplyVsync(); // vsync applies immediately
		});

	interfaceLoader.RegisterBoolAction("set_showfps", [this](bool value)
		{
			context.settings.SetShowFps(value); // read by the app each frame
		});
}

void MenuState::ShowPanel(const std::string& panelId)
{
	std::unique_ptr<UI::Element> frame = interfaceLoader.LoadFromFile(MENU_DIRECTORY + "frame.json");

	UI::Element* slot = frame->FindByName("panel_slot");
	if (slot == nullptr)
		throw std::runtime_error("MenuState: frame.json must contain 'panel_slot'");

	slot->AddChild(interfaceLoader.LoadFromFile(MENU_DIRECTORY + panelId + ".json"));

	userInterface.SetContent(std::move(frame));

	if (panelId == "audio")
		SetupAudioPanel();
	else if (panelId == "graphics")
		SetupGraphicsPanel();
}

void MenuState::SetupAudioPanel()
{
	SetVolumeDisplay("sound_slider", "sound_value", context.settings.GetSoundVolume());
	SetVolumeDisplay("music_slider", "music_value", context.settings.GetMusicVolume());
}

void MenuState::SetVolumeDisplay(const std::string& sliderName, const std::string& labelName, int value)
{
	if (auto* slider = dynamic_cast<UI::Slider*>(userInterface.FindByName(sliderName)))
		slider->SetValue(static_cast<float>(value));

	if (auto* label = dynamic_cast<UI::Label*>(userInterface.FindByName(labelName)))
		label->SetText(std::to_string(value));
}

void MenuState::SetupGraphicsPanel()
{
	resolutions.clear();
	for (const sf::VideoMode& mode : sf::VideoMode::getFullscreenModes())
	{
		if (std::find(resolutions.begin(), resolutions.end(), mode.size) == resolutions.end())
			resolutions.push_back(mode.size);
	}

	const sf::Vector2u current(static_cast<unsigned int>(context.settings.GetResolutionWidth()),
		static_cast<unsigned int>(context.settings.GetResolutionHeight()));

	const auto found = std::find(resolutions.begin(), resolutions.end(), current);
	if (found != resolutions.end())
	{
		resolutionIndex = static_cast<int>(std::distance(resolutions.begin(), found));
	}
	else
	{
		resolutions.insert(resolutions.begin(), current);
		resolutionIndex = 0;
	}

	UpdateResolutionLabel();
	UpdateScreenModeLabel();
	UpdateResolutionRowEnabled();

	if (auto* vsync = dynamic_cast<UI::Checkbox*>(userInterface.FindByName("vsync_checkbox")))
		vsync->SetChecked(context.settings.GetVsync());
	if (auto* showFps = dynamic_cast<UI::Checkbox*>(userInterface.FindByName("showfps_checkbox")))
		showFps->SetChecked(context.settings.GetShowFps());
}

void MenuState::StepResolution(int direction)
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

void MenuState::StepScreenMode(int direction)
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

void MenuState::UpdateResolutionLabel()
{
	if (auto* label = dynamic_cast<UI::Label*>(userInterface.FindByName("resolution_value")))
		label->SetText(std::to_string(context.settings.GetResolutionWidth()) + " x "
			+ std::to_string(context.settings.GetResolutionHeight()));
}

void MenuState::UpdateScreenModeLabel()
{
	std::string text = "Borderless";
	switch (context.settings.GetScreenMode())
	{
	case ScreenMode::Fullscreen: text = "Fullscreen"; break;
	case ScreenMode::Borderless: text = "Borderless"; break;
	case ScreenMode::Window:     text = "Window"; break;
	}

	if (auto* label = dynamic_cast<UI::Label*>(userInterface.FindByName("screenmode_value")))
		label->SetText(text);
}

void MenuState::UpdateResolutionRowEnabled()
{
	const bool enabled = context.settings.GetScreenMode() != ScreenMode::Borderless;
	const sf::Color color = enabled ? sf::Color(226, 210, 255, 255) : sf::Color(120, 120, 120, 255);

	if (auto* caption = dynamic_cast<UI::Label*>(userInterface.FindByName("resolution_caption")))
		caption->SetColor(color);
	if (auto* value = dynamic_cast<UI::Label*>(userInterface.FindByName("resolution_value")))
		value->SetColor(color);
}

bool MenuState::IsSettingsPanel(const std::string& panelId) const
{
	return panelId == "audio" || panelId == "graphics";
}

void MenuState::GoBackPanel()
{
	if (panelStack.size() > 1)
	{
		panelStack.pop_back();
		ShowPanel(panelStack.back());
	}
}

void MenuState::UpdateSaveButtonTint()
{
	auto* save = dynamic_cast<UI::Button*>(userInterface.FindByName("save_button"));
	if (save == nullptr)
		return;

	const sf::Color clean(120, 200, 120, 255); // green: nothing to save
	const sf::Color dirty(230, 150, 80, 255);  // orange: unsaved changes
	save->SetBackgroundTint(context.settings.IsDirty() ? dirty : clean);
}

void MenuState::OpenUnsavedChangesDialog()
{
	context.stateMachine.Push(std::make_unique<ConfirmState>(context,
		"Warning!", "You have unsaved changes. Save them?",
		[this] { SaveAndGoBack(); },
		[this] { RevertAndGoBack(); }));
}

void MenuState::SaveAndGoBack()
{
	context.settings.Save(SETTINGS_PATH);
	context.graphics.ApplyGraphics();
	GoBackPanel();
}

void MenuState::RevertAndGoBack()
{
	context.settings.Revert();
	context.audioMixer.SetSoundVolume(context.settings.GetSoundVolume() / 10.0f);
	context.audioMixer.SetMusicVolume(context.settings.GetMusicVolume() / 10.0f);
	context.graphics.ApplyVsync(); // restore vsync (resolution/mode were not applied yet)
	GoBackPanel();
}

void MenuState::ApplyPendingNavigation()
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
			if (IsSettingsPanel(panelStack.back()) && context.settings.IsDirty())
				OpenUnsavedChangesDialog();
			else
				GoBackPanel();
		}
		break;

	case NavRequest::Save:
		context.settings.Save(SETTINGS_PATH);
		context.graphics.ApplyGraphics();
		break;

	case NavRequest::StartGame:
		context.stateMachine.Push(std::make_unique<GameState>(context, "data/levels/test_level.tmj"));
		break;

	case NavRequest::Exit:
		context.stateMachine.Clear();
		break;

	case NavRequest::None:
		break;
	}

	pendingRequest = NavRequest::None;
}

void MenuState::HandleEvent(const sf::Event& event)
{
	if (transition.GetMode() != Transition::Mode::Idle)
		return;

	userInterface.HandleEvent(event);
}

void MenuState::Update(float deltaTime)
{
	transition.Update(deltaTime);

	backdrop.Update(deltaTime);
	userInterface.Update(deltaTime);

	if (transition.GetMode() == Transition::Mode::Idle)
	{
		Input& input = context.input;
		const bool activated = userInterface.IsElementActivated();

		if (input.WasPressed(Action::MenuBack))
		{
			if (activated)
				userInterface.CancelActivation();
			else
				pendingRequest = NavRequest::Back;
		}
		else if (!activated && input.WasPressed(Action::MenuDown))
		{
			userInterface.NavigateNext();
		}
		else if (!activated && input.WasPressed(Action::MenuUp))
		{
			userInterface.NavigatePrevious();
		}

		if (activated)
		{
			if (input.WasPressed(Action::MenuLeft))
				userInterface.NavigateValue(-1);
			else if (input.WasPressed(Action::MenuRight))
				userInterface.NavigateValue(1);
		}

		if (input.WasPressed(Action::MenuConfirm))
			userInterface.Confirm(true);
		else if (input.WasReleased(Action::MenuConfirm))
			userInterface.Confirm(false);

		UpdateSaveButtonTint();
		ApplyPendingNavigation();
	}
}

void MenuState::Render(float interpolationFactor)
{
	backdrop.Render(interpolationFactor);

	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	userInterface.Draw(context.virtualScreen.GetRenderTarget());
	transition.Draw(context.virtualScreen.GetRenderTarget());
}