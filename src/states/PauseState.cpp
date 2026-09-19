#include "PauseState.h"

#include "Context.h"
#include "core/Input.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "localization/LocalizationManager.h"
#include "states/GameState.h"
#include "states/MenuState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <memory>

namespace
{
	const std::string PauseUiPath = "data/ui/menu/pause.json";
}

PauseState::PauseState(Context& context, std::string levelPath, int levelNumber)
	: State(context, /*isRenderingStateBelow=*/true, /*isUpdatingStateBelow=*/false)
	, pauseInterface(context.virtualScreen)
	, pauseLoader(context.resources)
	, settings(context)
	, levelPath(std::move(levelPath))
	, levelNumber(levelNumber)
{
	pauseLoader.SetButtonSounds(context.audioMixer, "ui_hover", "ui_press");
	pauseLoader.SetLocalization(context.localization);
	lastLocalizationRevision = context.localization.Revision();
	RegisterActions();

	pauseInterface.SetContent(pauseLoader.LoadFromFile(PauseUiPath));
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
	if (isInSettings)
	{
		settings.HandleEvent(event);
		return;
	}

	pauseInterface.HandleEvent(event);
}

void PauseState::Update(float deltaTime)
{
	if (isInSettings)
	{
		settings.Update(deltaTime);

		if (settings.WasCloseRequested())
		{
			isInSettings = false;

			// pause.json was built before the visit (possibly in a different
			// language); Settings itself always reloads its own panels on a
			// language change, but pauseInterface was never told to.
			if (context.localization.Revision() != lastLocalizationRevision)
			{
				lastLocalizationRevision = context.localization.Revision();
				pauseInterface.SetContent(pauseLoader.LoadFromFile(PauseUiPath));
			}

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
		isInSettings = true;
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
	context.virtualScreen.SetCameraCenter(VirtualScreen::Width / 2.0f, VirtualScreen::Height / 2.0f);
	sf::RectangleShape overlay({ static_cast<float>(VirtualScreen::Width), static_cast<float>(VirtualScreen::Height) });
	overlay.setFillColor(sf::Color(0, 0, 0, 150));
	renderTarget.draw(overlay);

	if (isInSettings)
	{
		settings.Render(renderTarget);
		context.virtualScreen.CompositeGlow(VirtualScreen::GlowUiStrength);
		return;
	}

	pauseInterface.Draw(renderTarget);

	// Bloom the highlighted button.
	context.virtualScreen.CompositeGlow(VirtualScreen::GlowUiStrength);
}
