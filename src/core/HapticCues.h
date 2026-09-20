#pragma once

#include "core/GamepadHaptics.h"

#include <algorithm>

// Design-time feel tuning for menu vibration, gathered in one place so every
// call site asks for the same handful of named cues instead of picking its
// own motor/duration numbers. Not exposed to players; tune the constants
// below directly. Values are in the same spirit (and rough scale) as the
// "menu_navigation"/"title_letter_*" tuning table in this project's other
// title (Tessera), which loads them from JSON -- inlined here instead since
// there is no comparable per-player haptics settings surface in this project.
namespace Haptics
{
	// A light tick for moving focus between menu items (button/stepper hover,
	// list navigation). Both motors equally: at this strength there's no
	// perceptible difference between "left"/"right" feel, just a soft pulse.
	constexpr float NavigationMotor = 0.10f;
	constexpr float NavigationDuration = 0.05f;

	inline void PulseNavigation(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(NavigationMotor, NavigationMotor, NavigationDuration);
	}

	// A firmer thump for confirming something: pressing a button, or picking
	// an entry from a list (e.g. the language picker).
	constexpr float PressMotor = 0.35f;
	constexpr float PressDuration = 0.07f;

	inline void PulsePress(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(PressMotor, PressMotor, PressDuration);
	}

	// One title letter landing in the "Wild Adventure" drop-in animation.
	// letterFraction is 0 for the first letter and 1 for the last, so the
	// pulse grows steadily stronger as the word finishes assembling itself.
	constexpr float TitleLetterBaseMotor = 0.12f;
	constexpr float TitleLetterGrowMotor = 0.40f;
	constexpr float TitleLetterDuration = 0.05f;

	inline void PulseTitleLetterLanded(GamepadHaptics& haptics, float letterFraction)
	{
		const float motor = TitleLetterBaseMotor + TitleLetterGrowMotor * std::clamp(letterFraction, 0.f, 1.f);
		haptics.PulseVibration(motor, motor, TitleLetterDuration);
	}

	// A single key/button press on the splash screen's "press any key" prompt.
	constexpr float PromptMotor = 0.12f;
	constexpr float PromptDuration = 0.05f;

	inline void PulsePrompt(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(PromptMotor, PromptMotor, PromptDuration);
	}

	// A carousel arrow (Settings' resolution/screen-mode/language steppers):
	// fixed strength regardless of position, on the motor matching the
	// direction pressed -- XInput's low-frequency motor sits physically on
	// the left of the pad, the high-frequency one on the right.
	constexpr float CarouselMotor = 0.18f;
	constexpr float CarouselDuration = 0.05f;

	inline void PulseCarousel(GamepadHaptics& haptics, int direction)
	{
		if (direction < 0)
			haptics.PulseVibration(CarouselMotor, 0.f, CarouselDuration);
		else if (direction > 0)
			haptics.PulseVibration(0.f, CarouselMotor, CarouselDuration);
	}

	// A volume slider (the only sliders in the game) moving one step left or
	// right. Same left/right motor convention as the carousel, but scaled by
	// the value the slider now shows -- newNormalizedValue is that value
	// mapped to 0..1, so a slider near empty barely buzzes on either side and
	// one near full buzzes noticeably harder.
	constexpr float SliderMinMotor = 0.06f;
	constexpr float SliderMaxMotor = 0.30f;
	constexpr float SliderDuration = 0.05f;

	inline void PulseSlider(GamepadHaptics& haptics, int direction, float newNormalizedValue)
	{
		const float motor = SliderMinMotor
			+ (SliderMaxMotor - SliderMinMotor) * std::clamp(newNormalizedValue, 0.f, 1.f);

		if (direction < 0)
			haptics.PulseVibration(motor, 0.f, SliderDuration);
		else if (direction > 0)
			haptics.PulseVibration(0.f, motor, SliderDuration);
	}
}
