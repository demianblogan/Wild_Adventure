#include "CompanySplashState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "states/SplashState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <cstdint>
#include <memory>

namespace
{
	const std::string TEXTURE_ID = "company_splash";
	const std::string TEXTURE_PATH = "assets/textures/other/alone_bull_company.png";
	const std::string MUSIC_ID = "company_splash";

	constexpr float FADE_IN_TIME = 1.0f;
	constexpr float HOLD_TIME = 2.0f;
	constexpr float FADE_OUT_TIME = 1.0f;
}

CompanySplashState::CompanySplashState(Context& context)
	: State(context)
{
	if (!context.resources.textures.Has(TEXTURE_ID))
	{
		context.resources.textures.Load(TEXTURE_ID, TEXTURE_PATH);
	}

	context.audioMixer.PlayMusic(MUSIC_ID);
	context.graphics.SetCursorVisible(false); // no cursor during the logo
}

void CompanySplashState::HandleEvent(const sf::Event& event)
{
	const bool anyInput =
		event.is<sf::Event::KeyPressed>() ||
		event.is<sf::Event::MouseButtonPressed>() ||
		event.is<sf::Event::JoystickButtonPressed>();

	if (anyInput)
		StartFadeOut();
}

void CompanySplashState::StartFadeOut()
{
	if (phase == Phase::FadeOut || phase == Phase::Finished)
		return;

	fadeOutStartAlpha = alpha; // fade out smoothly from wherever we are now
	phase = Phase::FadeOut;
	timer = 0.0f;
}

void CompanySplashState::Update(float deltaTime)
{
	timer += deltaTime;

	switch (phase)
	{
	case Phase::FadeIn:
		alpha = std::min(1.0f, timer / FADE_IN_TIME);
		if (timer >= FADE_IN_TIME)
		{
			phase = Phase::Hold;
			timer = 0.0f;
		}
		break;

	case Phase::Hold:
		alpha = 1.0f;
		if (timer >= HOLD_TIME)
			StartFadeOut();
		break;

	case Phase::FadeOut:
		alpha = fadeOutStartAlpha * std::max(0.0f, 1.0f - timer / FADE_OUT_TIME);
		if (timer >= FADE_OUT_TIME)
		{
			phase = Phase::Finished;
			Leave();
		}
		break;

	case Phase::Finished:
		break;
	}
}

void CompanySplashState::Leave()
{
	if (isLeaving)
		return;

	isLeaving = true;

	context.graphics.SetCursorVisible(true); // restore the cursor for the menu

	context.stateMachine.Clear();
	context.stateMachine.Push(std::make_unique<SplashState>(context));
}

void CompanySplashState::Render(float interpolationFactor)
{
	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);

	const sf::Texture& texture = context.resources.textures.Get(TEXTURE_ID);
	const sf::Vector2u textureSize = texture.getSize();

	sf::Sprite sprite(texture);
	sprite.setPosition({ 0.0f, 0.0f });
	sprite.setScale({
		static_cast<float>(VirtualScreen::WIDTH) / static_cast<float>(textureSize.x),
		static_cast<float>(VirtualScreen::HEIGHT) / static_cast<float>(textureSize.y) });

	// The virtual screen is cleared to black each frame, so fading the image's
	// alpha gives a smooth fade from / to black.
	const std::uint8_t opacity = static_cast<std::uint8_t>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f);
	sprite.setColor(sf::Color(255, 255, 255, opacity));

	context.virtualScreen.GetRenderTarget().draw(sprite);
}