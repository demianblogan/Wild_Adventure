#include "MenuState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "states/GameState.h"
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

	transition.StartReveal();
}

void MenuState::RegisterActions()
{
	interfaceLoader.RegisterAction("menu_open_play", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "play"; });
	interfaceLoader.RegisterAction("menu_open_single", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "single"; });
	interfaceLoader.RegisterAction("menu_open_author", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "author"; });
	interfaceLoader.RegisterAction("menu_open_settings", [this] { inSettings = true; settings.Open(); });
	interfaceLoader.RegisterAction("menu_back", [this] { pendingRequest = NavRequest::Back; });
	interfaceLoader.RegisterAction("menu_exit", [this] { pendingRequest = NavRequest::Exit; });
	interfaceLoader.RegisterAction("menu_start_game", [this] { pendingRequest = NavRequest::StartGame; });
}

void MenuState::ShowPanel(const std::string& panelId)
{
	std::unique_ptr<UI::Element> frame = interfaceLoader.LoadFromFile(MENU_DIRECTORY + "frame.json");

	UI::Element* slot = frame->FindByName("panel_slot");
	if (slot == nullptr)
		throw std::runtime_error("MenuState: frame.json must contain 'panel_slot'");

	slot->AddChild(interfaceLoader.LoadFromFile(MENU_DIRECTORY + panelId + ".json"));

	userInterface.SetContent(std::move(frame));
	userInterface.ResetFocus();
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
		context.stateMachine.Push(std::make_unique<GameState>(context, "data/levels/level_1.tmj", 1));
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
	else
	{
		context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
		userInterface.Draw(renderTarget);
	}

	transition.Draw(renderTarget);
}