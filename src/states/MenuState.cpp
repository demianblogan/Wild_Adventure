#include "MenuState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Campaign.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "states/ConfirmState.h"
#include "states/GameState.h"
#include "ui/Button.h"
#include "ui/Element.h"

#include <memory>
#include <stdexcept>

namespace
{
	const std::string MENU_DIRECTORY = "data/ui/menu/";
}

MenuState::MenuState(Context& context)
	: State(context)
	, backdrop(context)
	, userInterface(context.virtualScreen)
	, interfaceLoader(context.resources)
	, settings(context)
	, selectLevel(context)
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

	context.audioMixer.PlayMusic("menu_theme");

	// Drop any per-level color grading carried over from gameplay.
	context.virtualScreen.SetColorGrading({});

	transition.StartReveal();
}

void MenuState::RegisterActions()
{
	interfaceLoader.RegisterAction("menu_open_play", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "play"; });
	interfaceLoader.RegisterAction("menu_open_single", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "single"; });
	interfaceLoader.RegisterAction("menu_open_author", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "author"; });
	interfaceLoader.RegisterAction("menu_open_settings", [this] { inSettings = true; settings.Open(); });
	interfaceLoader.RegisterAction("menu_select_level", [this] { inSelectLevel = true; selectLevel.Open(); });
	interfaceLoader.RegisterAction("menu_back", [this] { pendingRequest = NavRequest::Back; });
	interfaceLoader.RegisterAction("menu_exit", [this] { pendingRequest = NavRequest::Exit; });
	interfaceLoader.RegisterAction("menu_start_game", [this] { pendingRequest = NavRequest::StartGame; });
	interfaceLoader.RegisterAction("menu_continue_game", [this] { pendingRequest = NavRequest::ContinueGame; });
	interfaceLoader.RegisterAction("menu_delete_saves", [this] { pendingRequest = NavRequest::DeleteSaves; });
}

void MenuState::ShowPanel(const std::string& panelId)
{
	std::unique_ptr<UI::Element> frame = interfaceLoader.LoadFromFile(MENU_DIRECTORY + "frame.json");

	UI::Element* slot = frame->FindByName("panel_slot");
	if (slot == nullptr)
		throw std::runtime_error("MenuState: frame.json must contain 'panel_slot'");

	slot->AddChild(interfaceLoader.LoadFromFile(MENU_DIRECTORY + panelId + ".json"));

	userInterface.SetContent(std::move(frame));

	if (panelId == "single")
		SetupSinglePanel();
	else if (panelId == "play")
		SetupPlayPanel();

	userInterface.ResetFocus();
}

void MenuState::DisableButton(const std::string& buttonName)
{
	const sf::Color disabledTint(110, 110, 110, 255);

	if (auto* button = dynamic_cast<UI::Button*>(userInterface.FindByName(buttonName)))
	{
		button->SetEnabled(false);
		button->SetBackgroundTint(disabledTint);
		button->SetForegroundColor(UI::InteractionState::Normal, disabledTint);
	}
}

void MenuState::SetupSinglePanel()
{
	// Continue and Select Level unlock once the first level is completed.
	if (context.campaign.GetHighestCompletedLevel() < 1)
	{
		DisableButton("continue_button");
		DisableButton("select_level_button");
	}
}

void MenuState::SetupPlayPanel()
{
	// Two Players waits for the multiplayer implementation.
	DisableButton("two_players_button");

	// Nothing to delete until at least one level is completed.
	if (!context.campaign.HasProgress())
		DisableButton("delete_saves_button");
}

void MenuState::GoBackPanel()
{
	if (panelStack.size() > 1)
	{
		panelStack.pop_back();
		ShowPanel(panelStack.back());
	}
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
		GoBackPanel();
		break;

	case NavRequest::StartGame:
		context.stateMachine.Push(std::make_unique<GameState>(context, Campaign::LevelPath(1), 1));
		break;

	case NavRequest::ContinueGame:
	{
		// The next level after the furthest completed one; when everything
		// available is already done, replay the furthest level.
		const int highest = context.campaign.GetHighestCompletedLevel();
		int nextLevel = highest + 1;

		if (!Campaign::LevelExists(nextLevel))
			nextLevel = highest;

		if (Campaign::LevelExists(nextLevel))
			context.stateMachine.Push(std::make_unique<GameState>(context, Campaign::LevelPath(nextLevel), nextLevel));
		break;
	}

	case NavRequest::DeleteSaves:
		context.stateMachine.Push(std::make_unique<ConfirmState>(context,
			"Warning!",
			"This will delete all your campaign\n"
			"progress and you will have to\n"
			"start over. Do you want this?",
			[this]
			{
				context.campaign.Reset();

				// Rebuild the panel so Delete Saves immediately turns grey.
				ShowPanel(panelStack.back());
			},
			nullptr));
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

	if (inSettings)
	{
		settings.HandleEvent(event);
		return;
	}

	if (inSelectLevel)
	{
		selectLevel.HandleEvent(event);
		return;
	}

	userInterface.HandleEvent(event);
}

void MenuState::Update(float deltaTime)
{
	transition.Update(deltaTime);

	// The menu backdrop keeps moving at all times, including while the settings are
	// open (the settings are a panel over the live menu, not a separate state).
	backdrop.Update(deltaTime);

	if (transition.GetMode() != Transition::Mode::Idle)
		return;

	if (inSettings)
	{
		settings.Update(deltaTime);

		if (settings.WantsClose())
		{
			inSettings = false;
			userInterface.ResetFocus();
		}

		return;
	}

	if (inSelectLevel)
	{
		selectLevel.Update(deltaTime);

		if (selectLevel.WantsClose())
		{
			inSelectLevel = false;
			userInterface.ResetFocus();
		}

		return;
	}

	userInterface.Update(deltaTime);

	Input& input = context.input;

	if (input.WasPressed(Action::MenuBack))
	{
		pendingRequest = NavRequest::Back;
	}
	else if (input.WasPressed(Action::MenuDown))
	{
		userInterface.NavigateDown();
	}
	else if (input.WasPressed(Action::MenuUp))
	{
		userInterface.NavigateUp();
	}
	else if (input.WasPressed(Action::MenuLeft))
	{
		userInterface.NavigateLeft();
	}
	else if (input.WasPressed(Action::MenuRight))
	{
		userInterface.NavigateRight();
	}

	if (input.WasPressed(Action::MenuConfirm))
		userInterface.Confirm(true);
	else if (input.WasReleased(Action::MenuConfirm))
		userInterface.Confirm(false);

	ApplyPendingNavigation();
}

void MenuState::Render(float interpolationFactor)
{
	backdrop.Render(interpolationFactor);

	sf::RenderTarget& renderTarget = context.virtualScreen.GetRenderTarget();

	if (inSettings)
		settings.Render(renderTarget);
	else if (inSelectLevel)
		selectLevel.Render(renderTarget);
	else
	{
		context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
		userInterface.Draw(renderTarget);
	}

	transition.Draw(renderTarget);
}