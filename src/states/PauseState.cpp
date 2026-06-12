#include "PauseState.h"

#include "Context.h"
#include "core/Input.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "states/GameState.h"
#include "states/MenuState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <memory>

namespace
{
	const std::string PAUSE_UI_PATH = "data/ui/menu/pause.json";
}

PauseState::PauseState(Context& context, std::string levelPath, int levelNumber)
	: State(context, /*rendersStateBelow=*/true, /*updatesStateBelow=*/false)
	, pauseInterface(context.virtualScreen)
	, pauseLoader(context.resources)
	, settings(context)
	, levelPath(std::move(levelPath))
	, levelNumber(levelNumber)
{
	pauseLoader.SetButtonSounds(context.audioMixer, "ui_hover", "ui_press");
	RegisterActions();

	pauseInterface.SetContent(pauseLoader.LoadFromFile(PAUSE_UI_PATH));
	pauseInterface.ResetFocus();
}

void PauseState::RegisterActions()
{
	pauseLoader.RegisterAction("pause_continue", [this] { pendingRequest = NavRequest::Continue; });
	pauseLoader.RegisterAction("pause_restart",  [this] { pendingRequest = NavRequest::Restart; });
	pauseLoader.RegisterAction("pause_options",  [this] { pendingRequest = NavRequest::Options; });
	pauseLoader.RegisterAction("pause_quit",     [this] { pendingRequest = NavRequest::QuitToMenu; });
}

void PauseState::HandleEvent(const sf::Event& event)
{
	if (inSettings)
	{
		settings.HandleEvent(event);
		return;
	}

	pauseInterface.HandleEvent(event);
}

void PauseState::Update(float deltaTime)
{
	if (inSettings)
	{
		settings.Update(deltaTime);

		if (settings.WantsClose())
		{
			inSettings = false;
			pauseInterface.ResetFocus();
		}

		return;
	}

	pauseInterface.Update(deltaTime);

	Input& input = context.input;

	if (input.WasPressed(Action::Pause) || input.WasPressed(Action::MenuBack))
	{
		pendingRequest = NavRequest::Continue;
	}
	else if (input.WasPressed(Action::MenuDown))
	{
		pauseInterface.NavigateDown();
	}
	else if (input.WasPressed(Action::MenuUp))
	{
		pauseInterface.NavigateUp();
	}
	else if (input.WasPressed(Action::MenuLeft))
	{
		pauseInterface.NavigateLeft();
	}
	else if (input.WasPressed(Action::MenuRight))
	{
		pauseInterface.NavigateRight();
	}

	if (input.WasPressed(Action::MenuConfirm))
		pauseInterface.Confirm(true);
	else if (input.WasReleased(Action::MenuConfirm))
		pauseInterface.Confirm(false);

	ApplyPendingNavigation();
}

void PauseState::ApplyPendingNavigation()
{
	switch (pendingRequest)
	{
	case NavRequest::Continue:
		context.stateMachine.Pop();
		break;

	case NavRequest::Restart:
		context.stateMachine.Pop();
		context.stateMachine.Pop();
		context.stateMachine.Push(std::make_unique<GameState>(context, levelPath, levelNumber));
		break;

	case NavRequest::Options:
		inSettings = true;
		settings.Open("pause_frame");
		break;

	case NavRequest::QuitToMenu:
		context.stateMachine.Clear();
		context.stateMachine.Push(std::make_unique<MenuState>(context));
		break;

	case NavRequest::None:
		break;
	}

	pendingRequest = NavRequest::None;
}

void PauseState::Render(float /*interpolationFactor*/)
{
	sf::RenderTarget& renderTarget = context.virtualScreen.GetRenderTarget();

	// Frost the level rendered below; the pause menu stays sharp on top.
	context.virtualScreen.BlurContents();

	// Semi-transparent overlay to dim the level behind the pause menu.
	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	sf::RectangleShape overlay({ static_cast<float>(VirtualScreen::WIDTH), static_cast<float>(VirtualScreen::HEIGHT) });
	overlay.setFillColor(sf::Color(0, 0, 0, 150));
	renderTarget.draw(overlay);

	if (inSettings)
	{
		settings.Render(renderTarget);
		return;
	}

	pauseInterface.Draw(renderTarget);
}
