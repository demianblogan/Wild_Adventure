#include "screens/TitleDropAnimation.h"

#include "core/Random.h"
#include "core/Resources.h"
#include "graphics/ScreenShake.h"
#include "ui/Animation.h"
#include "ui/Element.h"
#include "ui/Image.h"
#include "ui/Label.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Glyph.hpp>

#include <cmath>
#include <cstdint>
#include <memory>
#include <numbers>
#include <vector>

namespace
{
	constexpr const char* FontName = "gameTitle";
	constexpr unsigned int CharacterSize = 28;
	constexpr unsigned int HugeCharacterSize = 110; // ~4x rest size: "right in front of the camera"
	const sf::Color RestColor(247, 198, 70, 255);
	const sf::Color FlashColor(255, 250, 220, 255);
	const sf::Color OutlineColor(99, 54, 22, 255);
	constexpr float OutlineThickness = 3.0f;

	constexpr float FlyDuration = 0.5f;     // huge+tilted -> real size/place/upright
	constexpr float LetterStagger = 0.07f;  // delay between consecutive letters' launches
	constexpr float StartRotationMin = 25.0f;
	constexpr float StartRotationMax = 50.0f;

	constexpr float OvershootDistance = 3.0f; // how far a letter dips past rest on landing
	constexpr float OvershootDuration = 0.05f;
	constexpr float SpringBackDuration = 0.12f;

	constexpr float FlashDuration = 0.18f;

	constexpr float ShakeTraumaPerLetter = 0.22f;

	const std::string DustTextureId = "title_drop_dust";
	const std::string DustTexturePath = "assets/textures/other/dust_particle.png";
	constexpr int ParticleCount = 6;
	constexpr float ParticleStartSize = 5.0f;
	constexpr float ParticleMinDistance = 8.0f;
	constexpr float ParticleMaxDistance = 18.0f;
	constexpr float ParticleDuration = 0.3f;

	sf::Color LerpColor(sf::Color from, sf::Color to, float t)
	{
		const auto lerpChannel = [t](std::uint8_t a, std::uint8_t b)
			{
				return static_cast<std::uint8_t>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t);
			};

		return sf::Color(
			lerpChannel(from.r, to.r), lerpChannel(from.g, to.g),
			lerpChannel(from.b, to.b), lerpChannel(from.a, to.a));
	}

	// A burst of pixel dust scattering outward from the impact point; each
	// piece shrinks and fades as it flies out, then just sits there invisible
	// (one-shot VFX, negligible cost -- not worth a RemoveChild API for this).
	void EmitDustBurst(UI::Element& container, Resources& resources, float centerX, float baselineY)
	{
		if (!resources.textures.Has(DustTextureId))
			resources.textures.Load(DustTextureId, DustTexturePath);

		for (int p = 0; p < ParticleCount; p++)
		{
			const float angle = Random::Float(0.0f, 2.0f * std::numbers::pi_v<float>);
			const float distance = Random::Float(ParticleMinDistance, ParticleMaxDistance);
			const sf::Vector2f start(centerX, baselineY);
			const sf::Vector2f end = start + sf::Vector2f(std::cos(angle), std::sin(angle)) * distance;

			auto particle = std::make_unique<UI::Image>(resources);
			particle->SetTexture(DustTextureId);
			particle->anchor = { 0.0f, 0.0f };
			particle->pivot = { 0.5f, 0.5f };
			particle->offset = start;
			particle->size = { ParticleStartSize, ParticleStartSize };
			particle->SetColor(sf::Color(255, 255, 255, 220));

			UI::Image& added = static_cast<UI::Image&>(container.AddChild(std::move(particle)));

			added.AddAnimation(std::make_unique<UI::Animation>(
				start.x, end.x, ParticleDuration,
				UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
				[&added](float x) { added.offset.x = x; }));

			added.AddAnimation(std::make_unique<UI::Animation>(
				start.y, end.y, ParticleDuration,
				UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
				[&added](float y) { added.offset.y = y; }));

			added.AddAnimation(std::make_unique<UI::Animation>(
				ParticleStartSize, 0.0f, ParticleDuration,
				UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
				[&added](float s) { added.size = { s, s }; }));

			added.AddAnimation(std::make_unique<UI::Animation>(
				220.0f, 0.0f, ParticleDuration,
				UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
				[&added](float a) { added.SetAlpha(a / 255.0f); }));
		}
	}

	// Impact: a quick golden flash, then a small overshoot-and-spring-back
	// around the letter's real resting Y (restY -- letters don't all share
	// Y = 0, see the baseline comment in BuildTitleDropAnimation). Calls
	// onSettled once the spring-back finishes.
	void PlayLandingBounce(UI::Label& letter, float restY, std::function<void()> onSettled)
	{
		letter.AddAnimation(std::make_unique<UI::Animation>(
			0.0f, 1.0f, FlashDuration,
			UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
			[&letter](float t) { letter.SetColor(LerpColor(FlashColor, RestColor, t)); }));

		UI::Animation& overshoot = letter.AddAnimation(std::make_unique<UI::Animation>(
			restY, restY + OvershootDistance, OvershootDuration,
			UI::AnimationCurve::Linear, UI::AnimationLoop::Once,
			[&letter](float y) { letter.offset.y = y; }));

		overshoot.SetOnFinished([&letter, restY, onSettled = std::move(onSettled)]
			{
				UI::Animation& springBack = letter.AddAnimation(std::make_unique<UI::Animation>(
					restY + OvershootDistance, restY, SpringBackDuration,
					UI::AnimationCurve::EaseOut, UI::AnimationLoop::Once,
					[&letter](float y) { letter.offset.y = y; }));

				springBack.SetOnFinished(onSettled);
			});
	}
}

void BuildTitleDropAnimation(UI::Element& container, Resources& resources, ScreenShake& shake,
	const std::string& asciiText, std::function<void()> onAllLanded)
{
	const sf::Font& font = resources.fonts.Get(FontName);

	// Advance-based horizontal layout (no per-glyph bearing correction):
	// close enough for a short, stylized title, and this only ever renders
	// plain ASCII. Vertically, though, each glyph's own ink-top offset from
	// its baseline (bounds.position.y) has to be tracked explicitly: Label
	// anchors by ink-top, so leaving every letter at the same offset.y (as
	// a single multi-character Label would effectively do internally) makes
	// short letters (no ascender: a, e, n, v...) sit noticeably higher than
	// tall ones (l, d, A...) instead of sharing a baseline. Letters are
	// placed at offset.y = topY[i] - minTopY, i.e. relative to whichever
	// letter has the tallest ascender, so a real shared baseline falls out
	// of that automatically.
	std::vector<float> positions(asciiText.size());
	std::vector<float> advances(asciiText.size());
	std::vector<float> topY(asciiText.size());
	float cursorX = 0.0f;
	float minTopY = 0.0f;
	bool haveMinTopY = false;

	for (std::size_t i = 0; i < asciiText.size(); i++)
	{
		const char32_t codepoint = static_cast<unsigned char>(asciiText[i]);

		if (i > 0)
			cursorX += font.getKerning(static_cast<unsigned char>(asciiText[i - 1]), codepoint, CharacterSize);

		positions[i] = cursorX;

		const sf::Glyph glyph = font.getGlyph(codepoint, CharacterSize, false);
		advances[i] = glyph.advance;
		topY[i] = glyph.bounds.position.y;
		cursorX += advances[i];

		if (asciiText[i] != ' ' && (!haveMinTopY || topY[i] < minTopY))
		{
			minTopY = topY[i];
			haveMinTopY = true;
		}
	}

	container.size = { cursorX, static_cast<float>(CharacterSize) };

	std::size_t totalLetters = 0;
	for (char c : asciiText)
		if (c != ' ')
			totalLetters++;

	// Shared across every letter's own (independently-timed) completion
	// callback so the last one to settle can tell it was the last one.
	auto landedCount = std::make_shared<std::size_t>(0);

	// Every letter launches from the same spot -- the title block's own
	// center -- so they all visibly come from one shared point, as if flung
	// out from right in front of the camera.
	const sf::Vector2f flightOrigin = { container.size.x / 2.0f, container.size.y / 2.0f };

	for (std::size_t i = 0; i < asciiText.size(); i++)
	{
		if (asciiText[i] == ' ')
			continue;

		const float restY = topY[i] - minTopY;
		const sf::Vector2f restPos = { positions[i], restY };
		const float startRotation = Random::Float(StartRotationMin, StartRotationMax) * ((i % 2 == 0) ? 1.0f : -1.0f);

		auto letterPtr = std::make_unique<UI::Label>(resources, FontName);
		UI::Label& letter = static_cast<UI::Label&>(container.AddChild(std::move(letterPtr)));

		letter.anchor = { 0.0f, 0.0f };
		letter.pivot = { 0.0f, 0.0f };
		letter.offset = flightOrigin;
		letter.rotationDegrees = startRotation;
		letter.isGlowing = true;
		letter.SetOutlineColor(OutlineColor);
		letter.SetOutlineThickness(OutlineThickness);
		letter.SetColor(RestColor);
		letter.SetCharacterSize(HugeCharacterSize);
		letter.SetText(std::string(1, asciiText[i]));

		const float delay = static_cast<float>(i) * LetterStagger;
		const float centerX = positions[i] + advances[i] * 0.5f;
		const float baselineY = restY + static_cast<float>(CharacterSize);

		letter.AddAnimation(std::make_unique<UI::Animation>(
			flightOrigin.x, restPos.x, FlyDuration,
			UI::AnimationCurve::EaseOut, UI::AnimationLoop::Once,
			[&letter](float x) { letter.offset.x = x; }, delay));

		letter.AddAnimation(std::make_unique<UI::Animation>(
			flightOrigin.y, restPos.y, FlyDuration,
			UI::AnimationCurve::EaseOut, UI::AnimationLoop::Once,
			[&letter](float y) { letter.offset.y = y; }, delay));

		letter.AddAnimation(std::make_unique<UI::Animation>(
			startRotation, 0.0f, FlyDuration,
			UI::AnimationCurve::EaseOut, UI::AnimationLoop::Once,
			[&letter](float r) { letter.rotationDegrees = r; }, delay));

		UI::Animation& shrink = letter.AddAnimation(std::make_unique<UI::Animation>(
			static_cast<float>(HugeCharacterSize), static_cast<float>(CharacterSize), FlyDuration,
			UI::AnimationCurve::EaseOut, UI::AnimationLoop::Once,
			[&letter](float s) { letter.SetCharacterSize(static_cast<unsigned int>(s + 0.5f)); }, delay));

		shrink.SetOnFinished([&letter, &container, &resources, &shake, restY, centerX, baselineY, totalLetters, landedCount, onAllLanded]
			{
				shake.Add(ShakeTraumaPerLetter);
				EmitDustBurst(container, resources, centerX, baselineY);

				PlayLandingBounce(letter, restY, [landedCount, totalLetters, onAllLanded]
					{
						(*landedCount)++;
						if (*landedCount == totalLetters && onAllLanded)
							onAllLanded();
					});
			});
	}
}
