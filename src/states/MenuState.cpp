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
	const std::string MenuDirectory = "data/ui/menu/";
}

MenuState::MenuState(Context& context)
	: State(context)
	, backdrop(context)
	, userInterface(context.virtualScreen)
	, interfaceLoader(context.resources)
	, settings(context)
	, selectLevel(context)
	, characterSelect(context)
{
	// Picking a level routes through the character select screen instead of
	// launching the level directly.
	selectLevel.SetLaunchHandler([this](int level)
	{
		isInSelectLevel = false;
		OpenCharacterSelect(level);
	});

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
	interfaceLoader.RegisterAction("menu_open_settings", [this] { isInSettings = true; settings.Open(); });
	interfaceLoader.RegisterAction("menu_select_level", [this] { isInSelectLevel = true; selectLevel.Open(); });
	interfaceLoader.RegisterAction("menu_back", [this] { pendingRequest = NavRequest::Back; });
	interfaceLoader.RegisterAction("menu_exit", [this] { pendingRequest = NavRequest::Exit; });
	interfaceLoader.RegisterAction("menu_start_game", [this] { pendingRequest = NavRequest::StartGame; });
	interfaceLoader.RegisterAction("menu_continue_game", [this] { pendingRequest = NavRequest::ContinueGame; });
	interfaceLoader.RegisterAction("menu_delete_saves", [this] { pendingRequest = NavRequest::DeleteSaves; });
}

void MenuState::ShowPanel(const std::string& panelId)
{
	std::unique_ptr<UI::Element> frame = interfaceLoader.LoadFromFile(MenuDirectory + "frame.json");

	UI::Element* slot = frame->FindByName("panel_slot");
	if (slot == nullptr)
		throw std::runtime_error("MenuState: frame.json must contain 'panel_slot'");

	slot->AddChild(interfaceLoader.LoadFromFile(MenuDirectory + panelId + ".json"));

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
	// Nothing to delete until at least one level is completed.
	if (!context.campaign.HasProgress())
		DisableButton("delete_saves_button");
}

void MenuState::OpenCharacterSelect(int levelNumber)
{
	isInCharacterSelect = true;
	characterSelect.Open(levelNumber);
}

void MenuState::GoBackPanel()
{
	if (panelStack.size() > 1)
	{
		panelStack.pop_back();
		ShowPanel(panelStack.back());
	}
	else
	{
		// Backing out of the root panel means leaving the game.
		OpenQuitDialog();
	}
}

void MenuState::OpenQuitDialog()
{
	context.stateMachine.Push(std::make_unique<ConfirmState>(context,
		"Warning!", "Do you want to quit the game?",
		[this] { context.stateMachine.Clear(); },
		nullptr));
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
		OpenCharacterSelect(1);
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
			OpenCharacterSelect(nextLevel);
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
		OpenQuitDialog();
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

	if (isInSettings)
	{
		settings.HandleEvent(event);
		return;
	}

	if (isInCharacterSelect)
	{
		characterSelect.HandleEvent(event);
		return;
	}

	if (isInSelectLevel)
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

	if (isInSettings)
	{
		settings.Update(deltaTime);

		if (settings.WantsClose())
		{
			isInSettings = false;
			userInterface.ResetFocus();
		}

		return;
	}

	if (isInCharacterSelect)
	{
		characterSelect.Update(deltaTime);

		if (characterSelect.WantsClose())
		{
			isInCharacterSelect = false;
			userInterface.ResetFocus();
		}

		return;
	}

	if (isInSelectLevel)
	{
		selectLevel.Update(deltaTime);

		if (selectLevel.WantsClose())
		{
			isInSelectLevel = false;
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

	// Bloom the backdrop's fruits now so their halos get frosted with it.
	context.virtualScreen.CompositeGlow();

	// Frost the level backdrop; the menu UI drawn on top stays sharp.
	context.virtualScreen.BlurContents();

	sf::RenderTarget& renderTarget = context.virtualScreen.GetRenderTarget();

	if (isInSettings)
		settings.Render(renderTarget);
	else if (isInCharacterSelect)
		characterSelect.Render(renderTarget);
	else if (isInSelectLevel)
		selectLevel.Render(renderTarget);
	else
	{
		context.virtualScreen.SetCameraCenter(VirtualScreen::Width / 2.0f, VirtualScreen::Height / 2.0f);
		userInterface.Draw(renderTarget);
	}

	// Bloom the highlighted button.
	context.virtualScreen.CompositeGlow(VirtualScreen::GlowUiStrength);

	transition.Draw(renderTarget);
}