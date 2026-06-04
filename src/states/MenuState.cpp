#include "MenuState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "ui/Element.h"

#include <SFML/Graphics/Font.hpp>

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
{
	Resources& resources = context.resources;

	// Works even if Menu is the first state (font not yet loaded by Splash).
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
	interfaceLoader.RegisterAction("menu_open_author", [this] { pendingRequest = NavRequest::OpenPanel; pendingPanelId = "author"; });
	interfaceLoader.RegisterAction("menu_back", [this] { pendingRequest = NavRequest::Back; });
	interfaceLoader.RegisterAction("menu_exit", [this] { pendingRequest = NavRequest::Exit; });
}

void MenuState::ShowPanel(const std::string& panelId)
{
	std::unique_ptr<UI::Element> frame = interfaceLoader.LoadFromFile(MENU_DIRECTORY + "frame.json");

	UI::Element* slot = frame->FindByName("panel_slot");
	if (slot == nullptr)
		throw std::runtime_error("MenuState: frame.json must contain 'panel_slot'");

	slot->AddChild(interfaceLoader.LoadFromFile(MENU_DIRECTORY + panelId + ".json"));

	userInterface.SetContent(std::move(frame));
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
			panelStack.pop_back();
			ShowPanel(panelStack.back());
		}
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
	// Buttons become clickable only after the entry wipe finishes.
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
		ApplyPendingNavigation();
}

void MenuState::Render(float interpolationFactor)
{
	backdrop.Render(interpolationFactor);

	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	userInterface.Draw(context.virtualScreen.GetRenderTarget());
	transition.Draw(context.virtualScreen.GetRenderTarget());
}