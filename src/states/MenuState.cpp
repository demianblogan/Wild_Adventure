#include "MenuState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Campaign.h"
#include "core/HapticCues.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "localization/LocalizationManager.h"
#include "states/ConfirmState.h"
#include "states/GameState.h"
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
		// Shares the button font's file: it is the only one of the three UI
		// fonts with Cyrillic glyphs, so "main" (used for most body text) has
		// to be backed by it too for Russian/Ukrainian to render at all.
		resources.fonts.Load("main", "assets/fonts/born2bsporty-fs.regular.otf");
		resources.fonts.Get("main").setSmooth(false);
	}

	if (!resources.fonts.Has("title"))
	{
		resources.fonts.Load("title", "assets/fonts/born2bsporty-fs.regular.otf");
		resources.fonts.Get("title").setSmooth(false);
	}

	if (!resources.fonts.Has("gameTitle"))
	{
		resources.fonts.Load("gameTitle", "assets/fonts/light-pixel-7.regular.ttf");
		resources.fonts.Get("gameTitle").setSmooth(false);
	}

	interfaceLoader.SetButtonSounds(context.audioMixer, "ui_hover", "ui_press");
	interfaceLoader.SetButtonHaptics(context.gamepadHaptics);
	interfaceLoader.SetLocalization(context.localization);
	lastLocalizationRevision = context.localization.Revision();

	Haptics::SetMenuLightbar(context.gamepadHaptics);

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
	interfaceLoader.RegisterAction("menu_open_credits", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "credits"; });
	interfaceLoader.RegisterAction("menu_open_settings", [this] { isInSettings = true; settings.Open(); });
	interfaceLoader.RegisterAction("menu_select_level", [this] { isInSelectLevel = true; selectLevel.Open(); });
	interfaceLoader.RegisterAction("menu_back", [this] { pendingRequest = NavRequest::Back; });
	interfaceLoader.RegisterAction("menu_exit", [this] { pendingRequest = NavRequest::Exit; });
	interfaceLoader.RegisterAction("menu_start_game", [this] { pendingRequest = NavRequest::StartGame; });
	interfaceLoader.RegisterAction("menu_continue_game", [this] { pendingRequest = NavRequest::ContinueGame; });
}

void MenuState::ShowPanel(const std::string& panelId)
{
	std::unique_ptr<UI::Element> frame = interfaceLoader.LoadFromFile(MenuDirectory + "frame.json");

	UI::Element* slot = frame->FindByName("panel_slot");
	if (slot == nullptr)
		throw std::runtime_error("MenuState: frame.json must contain 'panel_slot'");

	// Credits needs the full screen height for its block of text, so it hides
	// the frame's title and reclaims the space reserved above the panel slot.
	if (panelId == "credits")
	{
		if (UI::Element* title = frame->FindByName("title"))
			title->isVisible = false;

		slot->offset = { 0.0f, 0.0f };
	}

	slot->AddChild(interfaceLoader.LoadFromFile(MenuDirectory + panelId + ".json"));

	userInterface.SetContent(std::move(frame));

	if (panelId == "play")
		SetupPlayPanel();

	// Setup*Panel above may have just hidden a locked button; Root only
	// scans for navigable elements at SetContent time, so without this,
	// Up/Down (and mouse hover) could still land on something no longer
	// drawn.
	userInterface.RefreshInteractives();
}

void MenuState::SetupPlayPanel()
{
	// Continue and Select Level are meaningless until there is a level to
	// continue/select, so they're hidden entirely rather than shown disabled
	// -- New Game and Back re-center into the space that frees up.
	const bool hasProgress = context.campaign.HasProgress();

	if (UI::Element* button = userInterface.FindByName("continue_button"))
		button->isVisible = hasProgress;
	if (UI::Element* button = userInterface.FindByName("select_level_button"))
		button->isVisible = hasProgress;

	// New Game's own JSON position (-18) already doubles as its centered
	// spot when Continue/Select Level are hidden, so only Back needs to move.
	if (UI::Element* back = userInterface.FindByName("play_back_button"))
		back->offset.y = hasProgress ? 54.0f : 18.0f;
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
		context.localization.GetText("dialog.warning_title"),
		context.localization.GetText("dialog.quit_confirm_message"),
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
		if (context.campaign.HasProgress())
		{
			// New Game doubles as the only way to wipe progress now that the
			// separate Delete Progress button is gone, so starting fresh
			// over existing progress needs the same confirmation that used
			// to guard deleting it.
			context.stateMachine.Push(std::make_unique<ConfirmState>(context,
				context.localization.GetText("dialog.warning_title"),
				context.localization.GetText("dialog.delete_progress_message"),
				[this]
				{
					context.campaign.Reset();
					OpenCharacterSelect(1);
				},
				nullptr));
		}
		else
		{
			OpenCharacterSelect(1);
		}
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

		if (settings.WasCloseRequested())
		{
			isInSettings = false;

			// The menu behind Settings was built before the visit (possibly in
			// a different language); Settings itself always reloads its own
			// panels on a language change, but userInterface was never told to.
			if (context.localization.Revision() != lastLocalizationRevision)
			{
				lastLocalizationRevision = context.localization.Revision();
				ShowPanel(panelStack.back());
			}

			userInterface.ResetFocus();
		}

		return;
	}

	if (isInCharacterSelect)
	{
		characterSelect.Update(deltaTime);

		if (characterSelect.WasCloseRequested())
		{
			isInCharacterSelect = false;
			userInterface.ResetFocus();
		}

		return;
	}

	if (isInSelectLevel)
	{
		selectLevel.Update(deltaTime);

		if (selectLevel.WasCloseRequested())
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
		Haptics::PulsePress(context.gamepadHaptics);
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