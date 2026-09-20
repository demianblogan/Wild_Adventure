#pragma once

#include "core/State.h"
#include "graphics/ScreenShake.h"
#include "graphics/Transition.h"
#include "localization/Language.h"
#include "screens/MenuBackdrop.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace UI
{
	class Animation;
	class Label;
}

// Shown once, between CompanySplashState and the normal SplashState, when no
// language has been chosen yet (Settings::IsLanguageChosen() is false).
// Reuses the same backdrop and music as SplashState: the live backdrop dims
// and blurs behind a vertical list of five languages that animate in one
// after another (alternating from the left/right), while the heading text
// sharpens into focus over the same span. Hovering/focusing a language grows
// and softly pulses its text and previews the prompt in that language; once a
// language is picked, the list dissolves, the backdrop clears back up, and
// this turns in place into the ordinary splash screen (title + "press any
// key to continue"), then proceeds exactly like SplashState from there.
class LanguagePickerState : public State
{
public:
	LanguagePickerState(Context& context);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	enum class Phase { PickingLanguage, Dissolving, Splash };

	struct LanguageEntry
	{
		const char* buttonName;
		const char* labelName;
		Language language;
	};

	static const std::array<LanguageEntry, 5> LanguageEntries;

	void RegisterActions();
	void BuildPickerInterface();
	void BuildSplashInterface(); // mirrors SplashState::BuildInterface
	void WireLanguageButton(const LanguageEntry& entry, std::size_t index);
	void PreviewLanguage(Language language);
	void ConfirmLanguage(Language language);

	// Grows/pulses the newly highlighted language's text and shrinks the
	// previously highlighted one back down. Safe to call with the element
	// already highlighted (no-op) or with no prior highlight.
	void SetHighlightEmphasis(UI::Label* label);

	// Fades the list out along with the darkened/blurred backdrop, then hands
	// off to BuildSplashInterface once both finish.
	void PlayDissolveOut();

	// A handful of entrance/highlight animations need to start after a short
	// delay (staggering the language list); this is a tiny local scheduler so
	// Animation itself doesn't need a "delay" concept.
	void Schedule(float delay, std::function<void()> action);
	void UpdateSchedule(float deltaTime);

	struct DelayedCall
	{
		float remaining;
		std::function<void()> action;
	};

	MenuBackdrop backdrop;

	UI::Root userInterface;
	UI::DataLoader interfaceLoader;

	Transition transition;
	ScreenShake shake;

	Phase phase = Phase::PickingLanguage;
	bool isLeaving = false;

	// Set by the dissolve's last fade-out animation when it finishes. Acted
	// on from Update(), never from inside the callback itself: that callback
	// runs from inside the very Label's own Update() (by way of its
	// Animation::Update()), and BuildSplashInterface() tears down the whole
	// UI tree that Label belongs to -- destroying it out from under its own
	// still-running call stack. Deferring to the next Update() tick means the
	// tree is only ever replaced once nothing is mid-iteration over it.
	bool pendingSplashTransition = false;

	std::vector<DelayedCall> delayedCalls;

	// 0 = backdrop sharp and undarkened, 1 = fully dimmed/blurred. Driven by
	// darkenAnim; read every frame in Render() to pick the blur strength and
	// the darkening overlay's alpha.
	float backdropDarken = 0.0f;
	std::unique_ptr<UI::Animation> darkenAnim;

	UI::Label* highlightedLabel = nullptr;
};
