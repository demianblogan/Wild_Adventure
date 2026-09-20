#pragma once

#include "core/Camera.h"
#include "core/SceneLoader.h"
#include "core/State.h"
#include "core/ecs/Registry.h"
#include "graphics/ParticleSystem.h"
#include "graphics/ConfettiSystem.h"
#include "graphics/Transition.h"
#include "graphics/AnimatedBackground.h"
#include "graphics/LightOverlay.h"
#include "level/LevelSequencer.h"
#include "level/PlayerFeedbackController.h"
#include "level/ProgressSnapshot.h"
#include "systems/core/AnimationSystem.h"
#include "systems/enemies/BulletSystem.h"
#include "systems/core/DamageSystem.h"
#include "systems/core/DeathSystem.h"
#include "systems/enemies/EnemyDeathSystem.h"
#include "systems/enemies/EnemySystem.h"
#include "systems/enemies/GroundPatrolSystem.h"
#include "systems/core/InputSystem.h"
#include "systems/core/JumpSystem.h"
#include "systems/core/MovementSystem.h"
#include "systems/enemies/PatrolSystem.h"
#include "systems/core/PhysicsSystem.h"
#include "systems/traps/RockHeadSystem.h"
#include "systems/items/PickupSystem.h"
#include "systems/core/PlayerAnimationSystem.h"
#include "systems/core/RenderSystem.h"
#include "systems/items/BoxSystem.h"
#include "systems/traps/TrampolineSystem.h"
#include "systems/traps/ArrowSystem.h"
#include "systems/traps/FireSystem.h"
#include "systems/enemies/TrunkSystem.h"
#include "systems/enemies/PlantSystem.h"
#include "systems/enemies/BeeSystem.h"
#include "systems/enemies/ChickenSystem.h"
#include "systems/enemies/SnailSystem.h"
#include "systems/enemies/ShellSystem.h"
#include "systems/enemies/GhostSystem.h"
#include "systems/enemies/TurtleSystem.h"
#include "level/LevelSetup.h"
#include "screens/HUD.h"
#include "tilemap/Tilemap.h"

#include <SFML/System/Vector2.hpp>

#include <optional>
#include <string>
#include <vector>

class GameState : public State
{
public:
	GameState(Context& context, const std::string& levelPath, int levelNumber = 1,
		std::optional<sf::Vector2f> respawnOverride = std::nullopt,
		int initialScore = 0,
		std::optional<ProgressSnapshot> progressSnapshot = std::nullopt,
		int initialDeathCount = 0,
		int initialFruitsCollected = 0,
		int initialEnemiesKilled = 0);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	void InitScene(const nlohmann::json& mapJSON, std::optional<ProgressSnapshot> progressSnapshot);
	void SpawnPlayer();
	void UpdatePlayer(float deltaTime);

	bool IsPlayerOnDeathTile();

	ECS::Registry registry;
	SceneLoader sceneLoader;

	Camera camera;
	Tilemap tilemap;
	AnimatedBackground background;

	ParticleSystem particles;
	ConfettiSystem confetti;

	LightOverlay lightOverlay;
	LevelLighting lighting;

	int score = 0;

	int deathCount = 0;
	int fruitsCollected = 0;
	int enemiesKilled = 0;
	int maxFruits = 0;
	int maxEnemies = 0;

	ECS::InputSystem inputSystem;
	ECS::JumpSystem jumpSystem;
	ECS::DamageSystem damageSystem;
	ECS::DeathSystem deathSystem;
	ECS::PatrolSystem patrolSystem;
	ECS::EnemySystem enemySystem;
	ECS::TrunkSystem trunkSystem;
	ECS::PlantSystem plantSystem;
	ECS::BeeSystem beeSystem;
	ECS::ChickenSystem chickenSystem;
	ECS::SnailSystem snailSystem;
	ECS::ShellSystem shellSystem;
	ECS::GhostSystem ghostSystem;
	ECS::TurtleSystem turtleSystem;
	ECS::GroundPatrolSystem groundPatrolSystem;
	ECS::EnemyDeathSystem enemyDeathSystem;
	ECS::PhysicsSystem physicsSystem;
	ECS::RockHeadSystem rockHeadSystem;
	ECS::BoxSystem boxSystem;
	ECS::TrampolineSystem trampolineSystem;
	ECS::ArrowSystem arrowSystem;
	ECS::FireSystem fireSystem;
	ECS::MovementSystem movementSystem;
	ECS::BulletSystem bulletSystem;
	ECS::PickupSystem pickupSystem;
	ECS::AnimationSystem animationSystem;
	ECS::PlayerAnimationSystem playerAnimationSystem;
	ECS::RenderSystem renderSystem;

	HUD hud;

	Transition transition;

	// Level phase/checkpoint flow and the player's cosmetic reactions are each
	// their own class; GameState is left orchestrating which systems run, in
	// what order, and how a death or level completion transitions to the next
	// state.
	LevelSequencer levelSequencer;
	PlayerFeedbackController playerFeedback;

	std::string levelPath;
	int levelNumber = 1;
	float fallLimit = 0.0f;
	bool isRestarting = false;

	bool hasPlayedDeathSound = false;
	float deathFlashTimer = 0.0f; // white "lightning" flash over the background on death
	float deathFallTimer = 0.0f;  // time spent tumbling after a damage death

	bool  isWaterLevel = false;     // "water" theme: floaty gravity and ambient bubbles
	float bubbleTimer = 0.0f;     // countdown to the next ambient bubble

	// Low-health vignette: counts up continuously while the player is down to
	// the last heart (0 = off), driving a sine pulse; reset the instant that
	// stops being true so it always restarts the same fade-in next time.
	float lowHealthVignetteTime = 0.0f;

	static constexpr float LowHealthVignetteRadius = 200.0f;
	static constexpr float LowHealthVignetteSpeed = 6.0f; // radians/second
	static constexpr float LowHealthVignetteMinIntensity = 0.15f;
	static constexpr float LowHealthVignetteMaxIntensity = 0.45f;

	static constexpr float DeathFlashTime = 0.2f; // duration of the death "lightning" flash
	static constexpr float DeathFallTime = 0.5f;  // max tumble time before the restart kicks in
	static constexpr float WaterGravityScale = 0.55f;   // gravity multiplier in a water level
	static constexpr float WaterBubbleInterval = 0.15f; // seconds between ambient bubbles

	// Hit stop: the world freezes for a moment after stomping an enemy.
	float hitStopTimer = 0.0f;
	bool isHitStopFrozen = false; // interpolation already pinned for this freeze

	static constexpr float HitStopDuration = 0.06f;

	ECS::Entity playerEntity = ECS::InvalidEntity;
	ECS::Entity startPlatformEntity = ECS::InvalidEntity;

	std::optional<sf::Vector2f> respawnOverride; // set when reloading at a checkpoint

	static constexpr float AppearHeight = 50.0f;
};
