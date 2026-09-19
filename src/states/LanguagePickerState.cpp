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
#include "states/MenuState.h"
#include "ui/Animation.h"
#include "ui/Element.h"
#include "ui/InteractiveElement.h"
#include "ui/Label.h"

#include <memory>
#include <stdexcept>

namespace
{
	const std::string SettingsPath = AppDataPath::Resolve("settings.json").string();
}

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

	WireLanguageButton("lang_en", "lang_en_label", Language::English);
	WireLanguageButton("lang_de", "lang_de_label", Language::German);
	WireLanguageButton("lang_es", "lang_es_label", Language::Spanish);
	WireLanguageButton("lang_ru", "lang_ru_label", Language::Russian);
	WireLanguageButton("lang_uk", "lang_uk_label", Language::Ukrainian);

	// What the prompt shows before the player has hovered or focused
	// anything: English is also the button Root auto-focuses on load.
	PreviewLanguage(Language::English);
}

void LanguagePickerState::WireLanguageButton(const std::string& buttonName, const std::string& labelName, Language language)
{
	auto* button = dynamic_cast<UI::InteractiveElement*>(userInterface.FindByName(buttonName));
	auto* label = dynamic_cast<UI::Label*>(userInterface.FindByName(labelName));

	if (button == nullptr || label == nullptr)
		throw std::runtime_error("LanguagePickerState: language_picker.json is missing '" + buttonName + "'");

	label->SetText(context.localization.GetLanguageDisplayName(language));

	// Replaces the hover-sound-only callback SetButtonSounds wired: still
	// plays the sound, and also previews the prompt in this button's
	// language -- for both mouse hover and keyboard focus, since Root drives
	// both through the same SetHighlighted call.
	button->SetOnHighlighted([this, language]
		{
			context.audioMixer.PlaySound("ui_hover");
			PreviewLanguage(language);
		});
}

void LanguagePickerState::PreviewLanguage(Language language)
{
	if (auto* prompt = dynamic_cast<UI::Label*>(userInterface.FindByName("prompt")))
		prompt->SetText(context.localization.GetLanguagePickerPrompt(language));
}

void LanguagePickerState::ConfirmLanguage(Language language)
{
	context.settings.SetLanguage(language);
	context.settings.Save(SettingsPath);
	context.localization.SetLanguage(language);

	phase = Phase::Splash;
	BuildSplashInterface();
}

void LanguagePickerState::BuildSplashInterface()
{
	userInterface.SetContent(interfaceLoader.LoadFromFile("data/ui/splash.json"));

	UI::Element* title = userInterface.FindByName("title");
	UI::Label* prompt = dynamic_cast<UI::Label*>(userInterface.FindByName("prompt"));

	if (title == nullptr || prompt == nullptr)
		throw std::runtime_error("LanguagePickerState: splash.json must contain 'title' and 'prompt'");

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

void LanguagePickerState::HandleEvent(const sf::Event& event)
{
	if (transition.GetMode() != Transition::Mode::Idle)
		return;

	if (phase == Phase::PickingLanguage)
	{
		userInterface.HandleEvent(event); // mouse hover/click on the language buttons
		return;
	}

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

	if (phase == Phase::PickingLanguage)
	{
		Input& input = context.input;

		if (input.WasPressed(Action::MenuLeft))
			userInterface.NavigateLeft();
		else if (input.WasPressed(Action::MenuRight))
			userInterface.NavigateRight();
		else if (input.WasPressed(Action::MenuUp))
			userInterface.NavigateUp();
		else if (input.WasPressed(Action::MenuDown))
			userInterface.NavigateDown();

		if (input.WasPressed(Action::MenuConfirm))
			userInterface.Confirm(true);
		else if (input.WasReleased(Action::MenuConfirm))
			userInterface.Confirm(false);

		return;
	}

	if (transition.GetMode() == Transition::Mode::Done && !isLeaving)
	{
		isLeaving = true;
		context.stateMachine.Clear();
		context.stateMachine.Push(std::make_unique<MenuState>(context));
	}
}

void LanguagePickerState::Render(float interpolationFactor)
{
	backdrop.Render(interpolationFactor);

	// Bloom the backdrop's fruits at world strength.
	context.virtualScreen.CompositeGlow();

	// UI and transition: screen space, on top.
	context.virtualScreen.SetCameraCenter(VirtualScreen::Width / 2.0f, VirtualScreen::Height / 2.0f);
	userInterface.Draw(context.virtualScreen.GetRenderTarget());

	// Bloom the golden title (once it is showing) and highlighted buttons.
	context.virtualScreen.CompositeGlow(VirtualScreen::GlowUiStrength);

	transition.Draw(context.virtualScreen.GetRenderTarget());
}
