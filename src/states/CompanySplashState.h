#pragma once

#include "core/State.h"
#include "core/GraphicsTarget.h"

// The company logo shown at launch: fades in from black, holds, fades out, then
// hands over to the regular SplashState. A short jingle plays alongside it, and
// any key/mouse/gamepad press skips to the fade-out.
class CompanySplashState : public State
{
public:
	CompanySplashState(Context& context);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	enum class Phase
	{
		FadeIn,
		Hold,
		FadeOut,
		Finished
	};

	void StartFadeOut();
	void Leave();

	Phase phase = Phase::FadeIn;
	float timer = 0.0f;
	float alpha = 0.0f;             // current image opacity, 0..1
	float fadeOutStartAlpha = 1.0f; // opacity the fade-out starts from (for skipping mid-fade)
	bool isLeaving = false;
};