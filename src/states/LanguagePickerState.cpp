#include "LanguagePickerState.h"

#include "Context.h"
#include "audio/Mixer.h"
#include "core/AppDataPath.h"
#include "core/Input.h"
#include "core/Resources.h"
#include "core/Settings.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "localization/LocalizationManager.h"
#include "screens/TitleDropAnimation.h"
#include "states/MenuState.h"
#include "ui/Animation.h"
#include "ui/Element.h"
#include "ui/InteractiveElement.h"
#include "ui/Label.h"

#include <SFML/Graphics/RectangleShape.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace
{
	const std::string SettingsPath = AppDataPath::Resolve("settings.json").string();

	// Entrance timing: the heading sharpens over ItemsAnimDuration, and every
	// list item is timed to finish its own slide-in at that same instant too,
	// each one starting ItemStagger later than the one above it.
	constexpr float ItemsAnimDuration = 0.8f;
	constexpr float ItemStagger = 0.08f;
	constexpr float SlideDistance = 60.0f;

	// born2bsporty is a pixel font: it only stays crisp at the sizes it's
	// actually hinted for. 16 and 24 are both already used elsewhere in the
	// UI (buttons, headings) and render cleanly; an arbitrary in-between size
	// like 22 rasterizes with visibly blurred edges, so the highlighted size
	// has to land on one of those known-good sizes rather than anything else.
	constexpr unsigned int NormalCharSize = 16;
	constexpr unsigned int HighlightedCharSize = 24;
	constexpr float HighlightTweenDuration = 0.18f;
	constexpr float PulseDuration = 0.75f;
	constexpr float PulseMinAlpha = 0.55f;

	constexpr float PromptStartOutline = 5.0f;
	constexpr float PromptRestOutline = 1.0f;
	constexpr unsigned int PromptStartCharSize = 30;
	constexpr unsigned int PromptRestCharSize = 24;

	constexpr float DarkenFadeInDuration = 0.35f;
	constexpr float DarkenMaxAlpha = 120.0f; // out of 255
	constexpr int BlurIterations = 3;

	constexpr float DissolveDuration = 0.4f;
}

const std::array<LanguagePickerState::LanguageEntry, 5> LanguagePickerState::LanguageEntries =
{ {
	{ "lang_en", "lang_en_label", Language::English },
	{ "lang_de", "lang_de_label", Language::German },
	{ "lang_es", "lang_es_label", Language::Spanish },
	{ "lang_ru", "lang_ru_label", Language::Russian },
	{ "lang_uk", "lang_uk_label", Language::Ukrainian },
} };

LanguagePickerState::LanguagePickerState(Context& context)
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

	interfaceLoader.SetButtonSounds(context.audioMixer, "ui_hover", "ui_press");
	interfaceLoader.SetLocalization(context.localization);

	RegisterActions();
	BuildPickerInterface();

	context.audioMixer.PlayMusic("menu_theme");

	transition.StartReveal();
}

void LanguagePickerState::RegisterActions()
{
	interfaceLoader.RegisterAction("pick_en", [this] { ConfirmLanguage(Language::English); });
	interfaceLoader.RegisterAction("pick_de", [this] { ConfirmLanguage(Language::German); });
	interfaceLoader.RegisterAction("pick_es", [this] { ConfirmLanguage(Language::Spanish); });
	interfaceLoader.RegisterAction("pick_ru", [this] { ConfirmLanguage(Language::Russian); });
	interfaceLoader.RegisterAction("pick_uk", [this] { ConfirmLanguage(Language::Ukrainian); });
}

void LanguagePickerState::BuildPickerInterface()
{
	userInterface.SetContent(interfaceLoader.LoadFromFile("data/ui/menu/language_picker.json"));

	backdropDarken = 0.0f;
	darkenAnim = std::make_unique<UI::Animation>(
		0.0f, 1.0f, DarkenFadeInDuration,
		UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
		[this](float v) { backdropDarken = v; });

	// The heading starts soft/out-of-focus (dim, oversized, heavy outline)
	// and sharpens into place over the same span the list takes to settle.
	auto* prompt = dynamic_cast<UI::Label*>(userInterface.FindByName("prompt"));
	if (prompt == nullptr)
		throw std::runtime_error("LanguagePickerState: language_picker.json must contain 'prompt'");

	prompt->SetAlpha(0.0f);
	prompt->SetOutlineThickness(PromptStartOutline);
	prompt->SetCharacterSize(PromptStartCharSize);

	prompt->AddAnimation(std::make_unique<UI::Animation>(
		0.0f, 1.0f, ItemsAnimDuration,
		UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
		[prompt](float a) { prompt->SetAlpha(a); }));

	prompt->AddAnimation(std::make_unique<UI::Animation>(
		PromptStartOutline, PromptRestOutline, ItemsAnimDuration,
		UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
		[prompt](float t) { prompt->SetOutlineThickness(t); }));

	// Settles once it reaches PromptRestCharSize and stays put: at this small
	// a virtual resolution, continuously resizing a pixel font is not smooth
	// (each frame lands on a visibly different rasterization, no in-between),
	// so unlike the alpha pulse elsewhere, size is a one-shot sharpen only.
	prompt->AddAnimation(std::make_unique<UI::Animation>(
		static_cast<float>(PromptStartCharSize), static_cast<float>(PromptRestCharSize), ItemsAnimDuration,
		UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
		[prompt](float s) { prompt->SetCharacterSize(static_cast<unsigned int>(s + 0.5f)); }));

	for (std::size_t i = 0; i < LanguageEntries.size(); i++)
		WireLanguageButton(LanguageEntries[i], i);

	// What the prompt shows before the player has hovered or focused
	// anything: English is also the button Root auto-focuses on load, but
	// Root's initial focus is silent (no onHighlighted callback), so both the
	// prompt preview and the highlight emphasis are seeded here manually.
	PreviewLanguage(Language::English);

	if (auto* englishLabel = dynamic_cast<UI::Label*>(userInterface.FindByName("lang_en_label")))
	{
		// Deferred so the auto-focused item still plays its own entrance
		// fade/slide instead of jumping straight to the highlighted look;
		// only applies if the player hasn't already moved focus by then.
		Schedule(ItemsAnimDuration, [this, englishLabel]
			{
				if (highlightedLabel == nullptr)
					SetHighlightEmphasis(englishLabel);
			});
	}
}

void LanguagePickerState::WireLanguageButton(const LanguageEntry& entry, std::size_t index)
{
	auto* interactive = dynamic_cast<UI::InteractiveElement*>(userInterface.FindByName(entry.buttonName));
	auto* label = dynamic_cast<UI::Label*>(userInterface.FindByName(entry.labelName));
	auto* element = dynamic_cast<UI::Element*>(interactive);

	if (interactive == nullptr || element == nullptr || label == nullptr)
		throw std::runtime_error(std::string("LanguagePickerState: language_picker.json is missing '") + entry.buttonName + "'");

	label->SetText(context.localization.GetLanguageDisplayName(entry.language));

	// The list already grows/pulses its own text on highlight; Root's usual
	// highlighted-control bloom would wash the text out mid-pulse, so skip it.
	interactive->bloomsWhenHighlighted = false;

	// Entrance: slide in from the left/right (alternating by position in the
	// list) and fade in, staggered so every item finishes at the same time.
	const float restX = element->offset.x;
	const bool fromLeft = (index % 2) == 0;
	element->offset.x = restX + (fromLeft ? -SlideDistance : SlideDistance);
	label->SetAlpha(0.0f);

	const float delay = static_cast<float>(index) * ItemStagger;
	const float duration = ItemsAnimDuration - delay;

	Schedule(delay, [element, label, restX, duration]
		{
			element->AddAnimation(std::make_unique<UI::Animation>(
				element->offset.x, restX, duration,
				UI::AnimationCurve::EaseOut, UI::AnimationLoop::Once,
				[element](float x) { element->offset.x = x; }));

			label->AddAnimation(std::make_unique<UI::Animation>(
				0.0f, 1.0f, duration,
				UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
				[label](float a) { label->SetAlpha(a); }));
		});

	// Replaces the hover-sound-only callback SetButtonSounds wired: still
	// plays the sound, and also previews the prompt in this button's
	// language and emphasizes its text -- for both mouse hover and keyboard
	// focus, since Root drives both through the same SetHighlighted call.
	Language language = entry.language;
	interactive->SetOnHighlighted([this, label, language]
		{
			context.audioMixer.PlaySound("ui_hover");
			PreviewLanguage(language);
			SetHighlightEmphasis(label);
		});
}

void LanguagePickerState::SetHighlightEmphasis(UI::Label* label)
{
	if (label == nullptr || highlightedLabel == label)
		return;

	if (highlightedLabel != nullptr)
	{
		UI::Label* previous = highlightedLabel;
		previous->ClearAnimations();
		previous->SetAlpha(1.0f);
		previous->AddAnimation(std::make_unique<UI::Animation>(
			static_cast<float>(HighlightedCharSize), static_cast<float>(NormalCharSize), HighlightTweenDuration,
			UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
			[previous](float s) { previous->SetCharacterSize(static_cast<unsigned int>(s + 0.5f)); }));
	}

	highlightedLabel = label;
	label->ClearAnimations();

	label->AddAnimation(std::make_unique<UI::Animation>(
		static_cast<float>(NormalCharSize), static_cast<float>(HighlightedCharSize), HighlightTweenDuration,
		UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
		[label](float s) { label->SetCharacterSize(static_cast<unsigned int>(s + 0.5f)); }));

	label->AddAnimation(std::make_unique<UI::Animation>(
		1.0f, PulseMinAlpha, PulseDuration,
		UI::AnimationCurve::Sine, UI::AnimationLoop::PingPong,
		[label](float a) { label->SetAlpha(a); }));
}

void LanguagePickerState::PreviewLanguage(Language language)
{
	if (auto* prompt = dynamic_cast<UI::Label*>(userInterface.FindByName("prompt")))
		prompt->SetText(context.localization.GetLanguagePickerPrompt(language));
}

void LanguagePickerState::ConfirmLanguage(Language language)
{
	if (phase != Phase::PickingLanguage)
		return;

	context.settings.SetLanguage(language);
	context.settings.Save(SettingsPath);
	context.localization.SetLanguage(language);

	phase = Phase::Dissolving;

	// Any still-pending staggered entrance/highlight callback closes over a
	// pointer into the current UI tree; BuildSplashInterface (fired once the
	// dissolve finishes) replaces that tree wholesale, so a call left to fire
	// afterwards would run against freed elements. Drop them all now.
	delayedCalls.clear();

	PlayDissolveOut();
}

void LanguagePickerState::PlayDissolveOut()
{
	auto* prompt = dynamic_cast<UI::Label*>(userInterface.FindByName("prompt"));

	std::vector<UI::Label*> labels;
	if (prompt != nullptr)
		labels.push_back(prompt);

	for (const auto& entry : LanguageEntries)
		if (auto* label = dynamic_cast<UI::Label*>(userInterface.FindByName(entry.labelName)))
			labels.push_back(label);

	for (UI::Label* label : labels)
		label->ClearAnimations();

	highlightedLabel = nullptr;

	darkenAnim = std::make_unique<UI::Animation>(
		backdropDarken, 0.0f, DissolveDuration,
		UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
		[this](float v) { backdropDarken = v; });

	for (std::size_t i = 0; i < labels.size(); i++)
	{
		UI::Label* label = labels[i];

		UI::Animation& fade = label->AddAnimation(std::make_unique<UI::Animation>(
			1.0f, 0.0f, DissolveDuration,
			UI::AnimationCurve::Sine, UI::AnimationLoop::Once,
			[label](float a) { label->SetAlpha(a); }));

		// Any one of these finishing at the same instant works; use the last.
		// Just flag it here -- see pendingSplashTransition's comment for why
		// BuildSplashInterface() must not run from inside this callback.
		if (i + 1 == labels.size())
			fade.SetOnFinished([this] { pendingSplashTransition = true; });
	}
}

void LanguagePickerState::BuildSplashInterface()
{
	std::unique_ptr<UI::Element> content = interfaceLoader.LoadFromFile("data/ui/splash.json");

	UI::Element* title = content->FindByName("title");
	UI::Label* prompt = dynamic_cast<UI::Label*>(content->FindByName("prompt"));

	if (title == nullptr || prompt == nullptr)
		throw std::runtime_error("LanguagePickerState: splash.json must contain 'title' and 'prompt'");

	prompt->isVisible = false;

	// Letters need to exist before SetContent() below: Root only collects
	// which elements bloom (isGlowing) at that point, so anything added
	// afterwards would never get the glow pass.
	BuildTitleDropAnimation(*title, context.resources, shake, "Wild Adventure",
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

void LanguagePickerState::HandleEvent(const sf::Event& event)
{
	if (transition.GetMode() != Transition::Mode::Idle)
		return;

	if (phase == Phase::PickingLanguage)
	{
		userInterface.HandleEvent(event); // mouse hover/click on the language buttons
		return;
	}

	if (phase == Phase::Dissolving)
		return;

	const bool anyInput =
		event.is<sf::Event::KeyPressed>() ||
		event.is<sf::Event::MouseButtonPressed>() ||
		event.is<sf::Event::JoystickButtonPressed>();

	if (anyInput)
		transition.StartCover();
}

void LanguagePickerState::Update(float deltaTime)
{
	transition.Update(deltaTime);

	backdrop.Update(deltaTime);
	userInterface.Update(deltaTime);
	shake.Update(deltaTime);

	if (darkenAnim)
	{
		darkenAnim->Update(deltaTime);
		if (darkenAnim->IsFinished())
			darkenAnim.reset();
	}

	UpdateSchedule(deltaTime);

	// Safe to tear down and rebuild the UI tree here: userInterface.Update()
	// above has fully returned, so nothing is mid-iteration over it anymore.
	if (pendingSplashTransition)
	{
		pendingSplashTransition = false;
		phase = Phase::Splash;
		BuildSplashInterface();
		return;
	}

	if (phase == Phase::PickingLanguage)
	{
		Input& input = context.input;

		if (input.WasPressed(Action::MenuUp))
			userInterface.NavigateUp();
		else if (input.WasPressed(Action::MenuDown))
			userInterface.NavigateDown();

		if (input.WasPressed(Action::MenuConfirm))
			userInterface.Confirm(true);
		else if (input.WasReleased(Action::MenuConfirm))
			userInterface.Confirm(false);

		return;
	}

	if (phase == Phase::Dissolving)
		return;

	if (transition.GetMode() == Transition::Mode::Done && !isLeaving)
	{
		isLeaving = true;
		context.stateMachine.Clear();
		context.stateMachine.Push(std::make_unique<MenuState>(context));
	}
}

void LanguagePickerState::Schedule(float delay, std::function<void()> action)
{
	if (delay <= 0.0f)
	{
		action();
		return;
	}

	delayedCalls.push_back({ delay, std::move(action) });
}

void LanguagePickerState::UpdateSchedule(float deltaTime)
{
	if (delayedCalls.empty())
		return;

	for (DelayedCall& call : delayedCalls)
		call.remaining -= deltaTime;

	std::vector<std::function<void()>> ready;
	for (DelayedCall& call : delayedCalls)
		if (call.remaining <= 0.0f)
			ready.push_back(std::move(call.action));

	delayedCalls.erase(
		std::remove_if(delayedCalls.begin(), delayedCalls.end(),
			[](const DelayedCall& call) { return call.remaining <= 0.0f; }),
		delayedCalls.end());

	for (auto& action : ready)
		action();
}

void LanguagePickerState::Render(float interpolationFactor)
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

	if (backdropDarken > 0.0f)
	{
		const int iterations = static_cast<int>(std::lround(BlurIterations * backdropDarken));
		if (iterations > 0)
			context.virtualScreen.BlurContents(iterations);

		sf::RectangleShape darken({ static_cast<float>(VirtualScreen::Width), static_cast<float>(VirtualScreen::Height) });
		darken.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(DarkenMaxAlpha * backdropDarken)));
		context.virtualScreen.GetRenderTarget().draw(darken);
	}

	userInterface.Draw(context.virtualScreen.GetRenderTarget());

	// Bloom the golden title (once it is showing) and highlighted language text.
	context.virtualScreen.CompositeGlow(VirtualScreen::GlowUiStrength);

	transition.Draw(context.virtualScreen.GetRenderTarget());
}
