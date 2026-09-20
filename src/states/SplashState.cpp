#include "SplashState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/HapticCues.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "screens/TitleDropAnimation.h"
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
	if (!context.resources.fonts.Has("main"))
	{
		// Shares the button font's file: it is the only one of the three UI
		// fonts with Cyrillic glyphs, so "main" (used for most body text) has
		// to be backed by it too for Russian/Ukrainian to render at all.
		context.resources.fonts.Load("main", "assets/fonts/born2bsporty-fs.regular.otf");
		context.resources.fonts.Get("main").setSmooth(false);
	}

	if (!context.resources.fonts.Has("title"))
	{
		context.resources.fonts.Load("title", "assets/fonts/born2bsporty-fs.regular.otf");
		context.resources.fonts.Get("title").setSmooth(false);
	}

	if (!context.resources.fonts.Has("gameTitle"))
	{
		context.resources.fonts.Load("gameTitle", "assets/fonts/light-pixel-7.regular.ttf");
		context.resources.fonts.Get("gameTitle").setSmooth(false);
	}

	interfaceLoader.SetLocalization(context.localization);
	BuildInterface();

	Haptics::SetMenuLightbar(context.gamepadHaptics);

	context.audioMixer.PlayMusic("menu_theme");

	transition.StartReveal();
}

void SplashState::BuildInterface()
{
	std::unique_ptr<UI::Element> content = interfaceLoader.LoadFromFile("data/ui/splash.json");

	UI::Element* title = content->FindByName("title");
	UI::Label* prompt = dynamic_cast<UI::Label*>(content->FindByName("prompt"));

	if (title == nullptr || prompt == nullptr)
		throw std::runtime_error("SplashState: splash.json must contain 'title' and 'prompt'");

	prompt->isVisible = false;

	// Letters need to exist before SetContent() below: Root only collects
	// which elements bloom (isGlowing) at that point, so anything added
	// afterwards would never get the glow pass.
	BuildTitleDropAnimation(*title, context.resources, shake, context.gamepadHaptics, "Wild Adventure",
		[prompt]()
		{
			prompt->isVisible = true;
			prompt->AddAnimation(std::make_unique<UI::Animation>(
				1.0f, 0.25f, 0.6f,
				UI::AnimationCurve::Sine, UI::AnimationLoop::PingPong,
				[prompt](float alpha) { prompt->SetAlpha(alpha); }));
		});

	userInterface.SetContent(std::move(content));
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
	{
		Haptics::PulsePrompt(context.gamepadHaptics);
		transition.StartCover();
	}
}

void SplashState::Update(float deltaTime)
{
	transition.Update(deltaTime);

	backdrop.Update(deltaTime);
	userInterface.Update(deltaTime);
	shake.Update(deltaTime);

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

	// Bloom the backdrop's fruits at world strength.
	context.virtualScreen.CompositeGlow();

	// UI and transition: screen space, on top. The shake offset only nudges
	// the camera for this pass, so the world backdrop above stays put while
	// the title/UI layer visibly kicks on each letter's landing.
	const sf::Vector2f shakeOffset = shake.GetOffset();
	context.virtualScreen.SetCameraCenter(
		VirtualScreen::Width / 2.0f + shakeOffset.x,
		VirtualScreen::Height / 2.0f + shakeOffset.y);
	userInterface.Draw(context.virtualScreen.GetRenderTarget());

	// Bloom the golden title.
	context.virtualScreen.CompositeGlow(VirtualScreen::GlowUiStrength);

	transition.Draw(context.virtualScreen.GetRenderTarget());
}