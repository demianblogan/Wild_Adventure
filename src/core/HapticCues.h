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
	// --- DualSense lightbar (menu) ----------------------------------------------
	// No-ops on Xbox (that hardware has no lightbar); GamepadHaptics already
	// handles that itself. Every menu screen sets this resting honey color once
	// when it opens (see e.g. MenuState/PauseState's constructors) and every
	// menu "click" below -- navigating, pressing, cancelling, doesn't matter
	// which -- flashes it briefly brighter, the same way a UI sound plays on
	// all of them alike.
	constexpr RGBColor MenuRestColor{ 230, 145, 20 };
	constexpr RGBColor MenuFlashColor{ 255, 210, 120 };
	constexpr float MenuFlashDuration = 0.15f;

	inline void SetMenuLightbar(GamepadHaptics& haptics)
	{
		haptics.SetLightbarColor(MenuRestColor);
	}

	inline void FlashMenuLightbar(GamepadHaptics& haptics)
	{
		haptics.PulseLightbar(MenuFlashColor, MenuFlashDuration);
	}

	// A light tick for moving focus between menu items (button/stepper hover,
	// list navigation). Both motors equally: at this strength there's no
	// perceptible difference between "left"/"right" feel, just a soft pulse.
	constexpr float NavigationMotor = 0.10f;
	constexpr float NavigationDuration = 0.05f;

	inline void PulseNavigation(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(NavigationMotor, NavigationMotor, NavigationDuration);
		FlashMenuLightbar(haptics);
	}

	// A firmer thump for confirming something: pressing a button, cancelling
	// out of a screen, or picking an entry from a list (e.g. the language
	// picker) -- any of these count as the same "click" for the lightbar.
	constexpr float PressMotor = 0.35f;
	constexpr float PressDuration = 0.07f;

	inline void PulsePress(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(PressMotor, PressMotor, PressDuration);
		FlashMenuLightbar(haptics);
	}

	// Toggling a checkbox: same light strength as menu navigation, just
	// under its own name so call sites read as what they mean.
	inline void PulseCheckboxToggled(GamepadHaptics& haptics)
	{
		PulseNavigation(haptics);
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
		FlashMenuLightbar(haptics);
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

		FlashMenuLightbar(haptics);
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

		FlashMenuLightbar(haptics);
	}

	// --- Gameplay --------------------------------------------------------------
	// The menu cues above are mostly symmetric (no left/right meaning); gameplay
	// cues instead deliberately pick a low/high motor blend per event: the
	// low-frequency motor reads as a deep, heavy thud (a solid hit, a body's
	// weight landing), the high-frequency one as a lighter, brighter buzz (a
	// spark, a boing, a scrape). Picking a different blend per cue is what
	// makes them read as distinct sensations rather than "the same rumble,
	// louder or quieter".

	// Taking damage: a heavy, low-dominant punch.
	constexpr float DamageLowMotor = 0.90f;
	constexpr float DamageHighMotor = 0.45f;
	constexpr float DamageDuration = 0.14f;

	inline void PulseDamage(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(DamageLowMotor, DamageHighMotor, DamageDuration);
	}

	// Dying/respawning: heavier and longer than a normal damage hit, so losing
	// the last heart reads as more significant than any single previous hit.
	constexpr float DeathLowMotor = 0.95f;
	constexpr float DeathHighMotor = 0.55f;
	constexpr float DeathDuration = 0.30f;

	inline void PulseDeath(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(DeathLowMotor, DeathHighMotor, DeathDuration);
	}

	// Landing on the ground: a soft, low-only thump -- much gentler than a hit.
	constexpr float LandLowMotor = 0.30f;
	constexpr float LandHighMotor = 0.05f;
	constexpr float LandDuration = 0.05f;

	inline void PulseLand(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(LandLowMotor, LandHighMotor, LandDuration);
	}

	// One running footstep: a very light, very short tap, timed to the same
	// cadence as the run-dust VFX (see PlayerFeedbackController) -- inspired
	// by Astro's Playroom, where every stride has its own faint kick.
	//
	// Deliberately on the HIGH-frequency motor, not the low one: the
	// low-frequency motor is a heavier, slower-spinning weight that reads as
	// a blunt "thunk" even at low strength and barely has time to spin up at
	// all within a ~35ms pulse, while the high-frequency motor is a much
	// lighter, faster-spinning one built for exactly this kind of fine,
	// buzzy texture (footsteps, reloads, menu ticks are the textbook use for
	// it). Using the low motor here is what made this read as "coarse".
	constexpr float FootstepMotor = 0.10f;
	constexpr float FootstepDuration = 0.035f;

	inline void PulseFootstep(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(0.f, FootstepMotor, FootstepDuration);
	}

	// Picking up a fruit: light and high-only, a little "sparkle" rather than
	// a thump.
	constexpr float CollectLowMotor = 0.04f;
	constexpr float CollectHighMotor = 0.28f;
	constexpr float CollectDuration = 0.05f;

	inline void PulseCollect(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(CollectLowMotor, CollectHighMotor, CollectDuration);
	}

	// Touching a checkpoint: a warm, balanced medium pulse -- a reassuring
	// confirmation, not an impact.
	constexpr float CheckpointLowMotor = 0.30f;
	constexpr float CheckpointHighMotor = 0.25f;
	constexpr float CheckpointDuration = 0.10f;

	inline void PulseCheckpoint(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(CheckpointLowMotor, CheckpointHighMotor, CheckpointDuration);
	}

	// A box taking a hit: a sharp, bright crack -- higher and shorter than a
	// damage hit so wood/metal reads differently from flesh.
	constexpr float BoxHitLowMotor = 0.55f;
	constexpr float BoxHitHighMotor = 0.65f;
	constexpr float BoxHitDuration = 0.10f;

	inline void PulseBoxHit(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(BoxHitLowMotor, BoxHitHighMotor, BoxHitDuration);
	}

	// Launching off a trampoline or arrow booster: strong but high-leaning, so
	// it reads as a springy "boing" rather than the low-dominant punch of
	// taking damage.
	constexpr float LaunchLowMotor = 0.55f;
	constexpr float LaunchHighMotor = 0.80f;
	constexpr float LaunchDuration = 0.14f;

	inline void PulseLaunch(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(LaunchLowMotor, LaunchHighMotor, LaunchDuration);
	}

	// Stomping an enemy: a quick, punchy low-leaning thump -- satisfying but
	// clearly lighter/friendlier than taking damage, since this is a win for
	// the player rather than something painful.
	constexpr float StompLowMotor = 0.50f;
	constexpr float StompHighMotor = 0.35f;
	constexpr float StompDuration = 0.09f;

	inline void PulseStomp(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(StompLowMotor, StompHighMotor, StompDuration);
	}

	// A rock-head trap slamming into a wall/floor/ceiling: an ambient
	// environmental thud, gentler than a hit landing on the player.
	constexpr float RockImpactLowMotor = 0.35f;
	constexpr float RockImpactHighMotor = 0.15f;
	constexpr float RockImpactDuration = 0.09f;

	inline void PulseRockImpact(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(RockImpactLowMotor, RockImpactHighMotor, RockImpactDuration);
	}

	// One beat of the finish-cup fanfare -- call three times, spaced out, from
	// the moment the player touches it.
	constexpr float FinishImpactLowMotor = 0.90f;
	constexpr float FinishImpactHighMotor = 0.55f;
	constexpr float FinishImpactDuration = 0.12f;

	inline void PulseFinishImpact(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(FinishImpactLowMotor, FinishImpactHighMotor, FinishImpactDuration);
	}

	// Stuck to a wall and sliding down it: call every frame for as long as it
	// lasts (PulseVibration's merge rule turns that into one continuous
	// scrape instead of a stutter) -- brighter/buzzier than it is heavy.
	constexpr float WallSlideLowMotor = 0.12f;
	constexpr float WallSlideHighMotor = 0.32f;
	constexpr float WallSlideDuration = 0.08f; // longer than one frame so re-arming never gaps

	inline void PulseWallSlide(GamepadHaptics& haptics)
	{
		haptics.PulseVibration(WallSlideLowMotor, WallSlideHighMotor, WallSlideDuration);
	}

	// --- DualSense lightbar (gameplay) ------------------------------------------
	// The resting lightbar tracks the player's hearts instead of staying on the
	// menu's honey: bright green at full health, orange down a heart, a
	// pulsing red (see HeartbeatPulser below) on the last one, fading to black
	// as the death animation plays. SetHealthLightbar is meant to be called
	// every frame from PlayerFeedbackController -- SetLightbarColor is cheap
	// to call repeatedly with the same value.
	constexpr RGBColor HealthFullColor{ 40, 220, 60 };
	constexpr RGBColor HealthMediumColor{ 255, 140, 20 };
	constexpr RGBColor HealthCriticalColor{ 200, 20, 20 };
	constexpr RGBColor HealthCriticalFlashColor{ 255, 70, 40 };
	constexpr RGBColor BlackColor{ 0, 0, 0 };

	[[nodiscard]] inline unsigned char LerpChannel(unsigned char from, unsigned char to, float t)
	{
		return static_cast<unsigned char>(static_cast<float>(from) + (static_cast<float>(to) - static_cast<float>(from)) * t);
	}

	[[nodiscard]] inline RGBColor LerpColor(RGBColor from, RGBColor to, float t)
	{
		t = std::clamp(t, 0.f, 1.f);
		return { LerpChannel(from.r, to.r, t), LerpChannel(from.g, to.g, t), LerpChannel(from.b, to.b, t) };
	}

	// A slow "thump-thump ... thump-thump" heartbeat, meant to run for as long
	// as `active` stays true (e.g. the player is down to their last heart).
	// Owned by whoever drives it and ticked every frame; stops and resets the
	// instant `active` goes false so it never leaves a half-finished beat
	// hanging or picks back up mid-pattern. Also briefly brightens the
	// lightbar on every beat, in lockstep with the vibration tap, so the
	// pulsing red reads as the same heartbeat rather than two unrelated cues.
	class HeartbeatPulser
	{
	public:
		void Update(float deltaTime, bool active, GamepadHaptics& haptics)
		{
			if (!active)
			{
				timer = 0.f;
				isSecondTap = false;
				return;
			}

			timer -= deltaTime;
			if (timer > 0.f)
				return;

			haptics.PulseVibration(Motor, Motor * 0.5f, TapDuration);
			haptics.PulseLightbar(HealthCriticalFlashColor, TapDuration);
			timer = isSecondTap ? GapAfterPair : GapBetweenTaps;
			isSecondTap = !isSecondTap;
		}

	private:
		float timer = 0.f;
		bool isSecondTap = false;

		static constexpr float Motor = 0.40f;
		static constexpr float TapDuration = 0.05f;
		static constexpr float GapBetweenTaps = 0.12f;
		static constexpr float GapAfterPair = 0.55f;
	};

	// currentHealth <= 0 is handled by DeathLightbarFader below instead, not
	// here -- there is no "0 hearts" resting color, only the fade to black.
	inline void SetHealthLightbar(GamepadHaptics& haptics, int currentHealth)
	{
		if (currentHealth >= 3)
			haptics.SetLightbarColor(HealthFullColor);
		else if (currentHealth == 2)
			haptics.SetLightbarColor(HealthMediumColor);
		else
			haptics.SetLightbarColor(HealthCriticalColor);
	}

	// Ticks the lightbar from the last-heart red down to black across
	// DeathFadeDuration once the player dies, so the death animation reads as
	// the heartbeat finally giving out. Reset() re-arms it for the next life
	// (a checkpoint respawn or level restart), so the same fade always plays
	// from full red again rather than continuing a partial one.
	class DeathLightbarFader
	{
	public:
		void Reset()
		{
			elapsed = 0.f;
		}

		void Update(float deltaTime, GamepadHaptics& haptics)
		{
			elapsed += deltaTime;
			const float t = std::clamp(elapsed / DeathFadeDuration, 0.f, 1.f);
			haptics.SetLightbarColor(LerpColor(HealthCriticalColor, BlackColor, t));
		}

	private:
		float elapsed = 0.f;

		static constexpr float DeathFadeDuration = 0.6f;
	};

	// --- DualSense lightbar (gameplay one-shots) --------------------------------
	constexpr RGBColor CollectFlashColor{ 40, 220, 220 };   // cyan, picking up a fruit
	constexpr RGBColor GoldFlashColor{ 255, 200, 60 };      // checkpoint / finish cup
	constexpr float CollectFlashDuration = 0.18f;
	constexpr float CheckpointFlashDuration = 0.25f;
	constexpr float FinishFlashDuration = 0.6f; // spread across all 3 blinks below
	constexpr int FinishFlashBlinks = 3;

	inline void FlashCollectLightbar(GamepadHaptics& haptics)
	{
		haptics.PulseLightbar(CollectFlashColor, CollectFlashDuration);
	}

	inline void FlashCheckpointLightbar(GamepadHaptics& haptics)
	{
		haptics.PulseLightbar(GoldFlashColor, CheckpointFlashDuration);
	}

	inline void FlashFinishLightbar(GamepadHaptics& haptics)
	{
		haptics.PulseLightbar(GoldFlashColor, FinishFlashDuration, FinishFlashBlinks);
	}
}
