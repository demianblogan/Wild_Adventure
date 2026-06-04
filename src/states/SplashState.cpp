#include "SplashState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "states/MenuState.h"
#include "ui/Animation.h"
#include "ui/Label.h"

#include <memory>
#include <stdexcept>

SplashState::SplashState(Context& context)
	: State(context)
	, backdrop(context)
	, userInterface(context.virtualScreen)
	, interfaceLoader(context.resources)
{
	context.resources.fonts.Load("main", "assets/fonts/main.ttf");
	context.resources.fonts.Get("main").setSmooth(false);

	BuildInterface();

	context.audioMixer.PlayMusic("menu_theme");

	transition.StartReveal();
}

void SplashState::BuildInterface()
{
	userInterface.SetContent(interfaceLoader.LoadFromFile("data/ui/splash.json"));

	UI::Element* title = userInterface.FindByName("title");
	UI::Label* prompt = dynamic_cast<UI::Label*>(userInterface.FindByName("prompt"));

	if (title == nullptr || prompt == nullptr)
		throw std::runtime_error("SplashState: splash.json must contain 'title' and 'prompt'");

	const float titleRestY = title->offset.y;
	const float titleStartY = -title->size.y - 10.0f;
	title->offset.y = titleStartY;

	prompt->isVisible = false;

	UI::Animation& slide = title->AddAnimation(std::make_unique<UI::Animation>(
		titleStartY, titleRestY, 1.6f,
		UI::AnimationCurve::EaseOut, UI::AnimationLoop::Once,
		[title](float y) { title->offset.y = y; }));

	slide.SetOnFinished([prompt]()
		{
			prompt->isVisible = true;
			prompt->AddAnimation(std::make_unique<UI::Animation>(
				1.0f, 0.25f, 0.6f,
				UI::AnimationCurve::Sine, UI::AnimationLoop::PingPong,
				[prompt](float alpha) { prompt->SetAlpha(alpha); }));
		});
}

void SplashState::HandleEvent(const sf::Event& event)
{
	if (transition.GetMode() != Transition::Mode::Idle)
		return;

	const bool anyInput =
		event.is<sf::Event::KeyPressed>() ||
		event.is<sf::Event::MouseButtonPressed>() ||
		event.is<sf::Event::JoystickButtonPressed>();

	if (anyInput)
		transition.StartCover();
}

void SplashState::Update(float deltaTime)
{
	transition.Update(deltaTime);

	backdrop.Update(deltaTime);
	userInterface.Update(deltaTime);

	if (transition.GetMode() == Transition::Mode::Done && !isLeaving)
	{
		isLeaving = true;
		context.stateMachine.Clear();
		context.stateMachine.Push(std::make_unique<MenuState>(context));
	}
}

void SplashState::Render(float interpolationFactor)
{
	backdrop.Render(interpolationFactor);

	// UI and transition: screen space, on top.
	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	userInterface.Draw(context.virtualScreen.GetRenderTarget());
	transition.Draw(context.virtualScreen.GetRenderTarget());
}