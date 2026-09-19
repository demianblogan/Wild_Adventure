#pragma once

#include <SFML/System/Vector2.hpp>

class Camera;
class ParticleSystem;

namespace Audio
{
	class Mixer;
}

namespace ECS
{
	struct CollisionState;
	struct Health;
	struct Jump;
	struct Sprite;
	struct Velocity;
}

// The player's purely cosmetic per-frame reactions: squash & stretch, run
// dust, jump/land/wall-slide VFX and SFX, and a camera shake + hurt sound on
// taking damage. None of this affects gameplay -- it only exists so GameState
// isn't also the one tracking every "previous frame" value this needs.
class PlayerFeedbackController
{
public:
	PlayerFeedbackController(Camera& camera, ParticleSystem& particles, Audio::Mixer& audioMixer);

	// Seeds the health baseline right after the player spawns, so the first
	// Update call never mistakes a fresh spawn for having taken damage.
	void ResetHealthBaseline(int maxHealth) { previousPlayerHealth = maxHealth; }

	// Reacts to the player's current physics/combat state and applies the
	// resulting squash scale to `sprite` (when the player has one).
	void Update(float deltaTime, sf::Vector2f feet, const ECS::Velocity& velocity,
		const ECS::CollisionState& collisionState, const ECS::Jump& jump,
		const ECS::Health& health, ECS::Sprite* sprite);

private:
	Camera& camera;
	ParticleSystem& particles;
	Audio::Mixer& audioMixer;

	bool wasOnGround = false;
	float runDustTimer = 0.0f;
	int previousJumpsRemaining = 0;
	float previousLockTimer = 0.0f;
	int previousPlayerHealth = -1;

	// Squash & stretch: the player's sprite briefly deforms on jump, land and
	// hit, then springs back to normal. X/Y pairs roughly preserve volume.
	float squashX = 1.0f;
	float squashY = 1.0f;

	static constexpr float RunDustInterval = 0.12f;
	static constexpr float SquashReturnSpeed = 10.0f; // exponential snap-back rate

	static constexpr sf::Vector2f SquashJump = { 0.80f, 1.25f }; // taking off: tall and thin
	static constexpr sf::Vector2f SquashLand = { 1.25f, 0.80f }; // touchdown: wide and short
	static constexpr sf::Vector2f SquashHitSide     = { 0.75f, 1.20f }; // compressed along the blow
	static constexpr sf::Vector2f SquashHitVertical = { 1.20f, 0.75f };

	static constexpr float ShakeHit = 0.75f; // camera trauma added on taking damage
};
