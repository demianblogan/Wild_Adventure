#pragma once

#include "core/State.h"
#include "graphics/Transition.h"
#include "localization/Language.h"
#include "screens/MenuBackdrop.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

#include <string>

// Shown once, between CompanySplashState and the normal SplashState, when no
// language has been chosen yet (Settings::IsLanguageChosen() is false).
// Reuses the same backdrop and music as SplashState: first a "choose your
// language" prompt with five language buttons (hovering/focusing one
// previews the prompt in that language); once a language is picked, this
// turns in place into the ordinary splash screen (title + "press any key
// to continue") with no visible transition, then proceeds exactly like
// SplashState from there.
class LanguagePickerState : public State
{
public:
	LanguagePickerState(Context& context);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	enum class Phase { PickingLanguage, Splash };

	void RegisterActions();
	void BuildPickerInterface();
	void BuildSplashInterface(); // mirrors SplashState::BuildInterface
	void WireLanguageButton(const std::string& buttonName, const std::string& labelName, Language language);
	void PreviewLanguage(Language language);
	void ConfirmLanguage(Language language);

	MenuBackdrop backdrop;

	UI::Root userInterface;
	UI::DataLoader interfaceLoader;

	Transition transition;

	Phase phase = Phase::PickingLanguage;
	bool isLeaving = false;
};
