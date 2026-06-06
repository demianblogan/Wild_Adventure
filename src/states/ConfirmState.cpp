#include "ConfirmState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Input.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "ui/Element.h"
#include "ui/Label.h"

#include <utility>

namespace
{
	const std::string DIALOG_PATH = "data/ui/menu/dialog.json";
}

ConfirmState::ConfirmState(Context& context, const std::string& title, const std::string& message,
	std::function<void()> onYes, std::function<void()> onNo)
	: State(context, true, false) // renders the menu below, but freezes it (modal)
	, dialog(context.virtualScreen)
	, loader(context.resources)
	, onYes(std::move(onYes))
	, onNo(std::move(onNo))
{
	loader.SetButtonSounds(context.audioMixer, "ui_hover", "ui_press");

	loader.RegisterAction("dialog_yes", [this] { if (this->onYes) this->onYes(); Close(); });
	loader.RegisterAction("dialog_no", [this] { if (this->onNo) this->onNo(); Close(); });

	dialog.SetContent(loader.LoadFromFile(DIALOG_PATH));

	if (auto* titleLabel = dynamic_cast<UI::Label*>(dialog.FindByName("dialog_title")))
		titleLabel->SetText(title);
	if (auto* messageLabel = dynamic_cast<UI::Label*>(dialog.FindByName("dialog_message")))
		messageLabel->SetText(message);
}

void ConfirmState::Close()
{
	if (closed)
		return;

	closed = true;
	context.stateMachine.Pop();
}

void ConfirmState::HandleEvent(const sf::Event& event)
{
	dialog.HandleEvent(event); // mouse
}

void ConfirmState::Update(float deltaTime)
{
	dialog.Update(deltaTime);

	Input& input = context.input;

	if (input.WasPressed(Action::MenuBack))
	{
		if (onNo)
			onNo();

		Close();

		return;
	}

	if (input.WasPressed(Action::MenuRight) || input.WasPressed(Action::MenuDown))
		dialog.NavigateNext();
	else if (input.WasPressed(Action::MenuLeft) || input.WasPressed(Action::MenuUp))
		dialog.NavigatePrevious();

	if (input.WasPressed(Action::MenuConfirm))
		dialog.Confirm(true);
	else if (input.WasReleased(Action::MenuConfirm))
		dialog.Confirm(false);
}

void ConfirmState::Render(float interpolationFactor)
{
	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	dialog.Draw(context.virtualScreen.GetRenderTarget());
}