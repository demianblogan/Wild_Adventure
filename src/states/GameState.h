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

struct ProgressSnapshot
{
	std::vector<sf::Vector2f> aliveCollectibles; // positions of uncollected fruits at checkpoint
	std::vector<sf::Vector2f> aliveBoxes;        // positions of unbroken boxes at checkpoint
	std::vector<sf::Vector2f> aliveEnemies;      // spawn positions of living enemies at checkpoint
};

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

	void UpdateLevelFlow(float deltaTime);
	void UpdateCheckpoints();

	bool IsPlayerOnStartPlatform();
	bool IsPlayerOnFinish();
	bool IsPlayerOnDeathTile();

	enum class LevelPhase
	{
		Revealing,
		Appearing,
		Playing,
		Finishing,
		Disappearing,
		Complete
	};

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
	int checkpointFruitsCollected = 0;
	int checkpointEnemiesKilled = 0;

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

	std::string levelPath;
	int levelNumber = 1;
	float fallLimit = 0.0f;
	bool isRestarting = false;

	bool wasOnGround = false;
	bool deathSoundPlayed = false;
	float deathFlashTimer = 0.0f; // white "lightning" flash over the background on death
	float deathFallTimer = 0.0f;  // time spent tumbling after a damage death

	bool  waterLevel = false;     // "water" theme: floaty gravity and ambient bubbles
	float bubbleTimer = 0.0f;     // countdown to the next ambient bubble

	static constexpr float DEATH_FLASH_TIME = 0.2f; // duration of the death "lightning" flash
	static constexpr float DEATH_FALL_TIME = 0.5f;  // max tumble time before the restart kicks in
	static constexpr float WATER_GRAVITY_SCALE = 0.55f;   // gravity multiplier in a water level
	static constexpr float WATER_BUBBLE_INTERVAL = 0.15f; // seconds between ambient bubbles

	int previousPlayerHealth = -1;
	int previousJumpsRemaining = 0;
	float runDustTimer = 0.0f;
	float previousLockTimer = 0.0f;

	static constexpr float RUN_DUST_INTERVAL = 0.12f;

	// Hit stop: the world freezes for a moment after stomping an enemy.
	float hitStopTimer = 0.0f;
	bool hitStopFrozen = false; // interpolation already pinned for this freeze

	static constexpr float HIT_STOP_DURATION = 0.06f;

	// Squash & stretch: the player's sprite briefly deforms on jump, land and
	// hit, then springs back to normal. X/Y pairs roughly preserve volume.
	float squashX = 1.0f;
	float squashY = 1.0f;

	static constexpr sf::Vector2f SQUASH_JUMP = { 0.80f, 1.25f }; // taking off: tall and thin
	static constexpr sf::Vector2f SQUASH_LAND = { 1.25f, 0.80f }; // touchdown: wide and short
	static constexpr sf::Vector2f SQUASH_HIT_SIDE     = { 0.75f, 1.20f }; // compressed along the blow
	static constexpr sf::Vector2f SQUASH_HIT_VERTICAL = { 1.20f, 0.75f };
	static constexpr float SQUASH_RETURN_SPEED = 10.0f; // exponential snap-back rate

	LevelPhase levelPhase = LevelPhase::Revealing;
	ECS::Entity playerEntity = ECS::INVALID_ENTITY;
	ECS::Entity startPlatformEntity = ECS::INVALID_ENTITY;
	ECS::Entity appearEffectEntity = ECS::INVALID_ENTITY;
	bool startMovingPlayed = false;

	ECS::Entity finishEntity = ECS::INVALID_ENTITY;
	ECS::Entity disappearEffectEntity = ECS::INVALID_ENTITY;
	float finishTimer = 0.0f;
	bool levelCompleteShown = false;

	std::optional<sf::Vector2f> respawnOverride;           // set when reloading at a checkpoint
	sf::Vector2f respawnPoint;                             // current respawn (start, or last checkpoint)
	int checkpointScore = 0;                               // score frozen at the last checkpoint touch
	std::optional<ProgressSnapshot> checkpointSnapshot;   // entity state at the last checkpoint touch

	static constexpr float APPEAR_HEIGHT = 50.0f;
	static constexpr float FINISH_RISE_TIME = 0.3f; // bounce arc before the hero vanishes
};