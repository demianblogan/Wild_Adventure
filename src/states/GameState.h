#pragma once

#include "core/Camera.h"
#include "core/DataLoader.h"
#include "core/State.h"
#include "core/ecs/Registry.h"
#include "graphics/ParticleSystem.h"
#include "graphics/ConfettiSystem.h"
#include "graphics/Transition.h"
#include "graphics/AnimatedBackground.h"
#include "graphics/LightOverlay.h"
#include "systems/AnimationSystem.h"
#include "systems/BulletSystem.h"
#include "systems/DamageSystem.h"
#include "systems/DeathSystem.h"
#include "systems/EnemyDeathSystem.h"
#include "systems/EnemySystem.h"
#include "systems/GroundPatrolSystem.h"
#include "systems/InputSystem.h"
#include "systems/JumpSystem.h"
#include "systems/MovementSystem.h"
#include "systems/PatrolSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/PickupSystem.h"
#include "systems/PlayerAnimationSystem.h"
#include "systems/RenderSystem.h"
#include "systems/BoxSystem.h"
#include "systems/TrampolineSystem.h"
#include "systems/TrunkSystem.h"
#include "systems/ChickenSystem.h"
#include "tilemap/Tilemap.h"
#include "ui/DataLoader.h"
#include "ui/Root.h"

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

// "Lamp" lighting of cave levels: the player sees a soft-edged circle around
// himself, everything else is dark. Loaded from data/levels/lighting.json.
struct LevelLighting
{
	bool enabled = false;
	float radius = 90.0f;   // fully dark at this distance from the player, in pixels
	float darkness = 0.95f; // shade outside the circle, 0..1 (1 = pure black)
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
	void UpdateScoreLabel();
	void UpdateHearts(int currentHealth, float deltaTime);
	void UpdateLevelFlow(float deltaTime);
	void UpdateCheckpoints();
	void UpdateLevelBanner(float deltaTime);
	void DrawLevelBanner();

	bool IsPlayerOnStartPlatform();
	bool IsPlayerOnFinish();
	bool IsPlayerOnDeathTile();

	sf::Vector2f PlayerCenter();

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
	DataLoader sceneLoader;

	Camera camera;
	Tilemap tilemap;
	AnimatedBackground background;

	ParticleSystem particles;
	ConfettiSystem confetti;

	LightOverlay lightOverlay;
	LevelLighting lighting;

	int score = 0;
	int previousScore = -1;
	int maxHearts = 3;

	int deathCount = 0;
	int fruitsCollected = 0;
	int enemiesKilled = 0;
	int maxFruits = 0;
	int maxEnemies = 0;
	int checkpointFruitsCollected = 0;
	int checkpointEnemiesKilled = 0;
	int displayedHealth = 3;
	int blinkingHeart = -1;
	float blinkTimer = 0.0f;

	static constexpr float HEART_BLINK_DURATION = 0.5f;

	ECS::InputSystem inputSystem;
	ECS::JumpSystem jumpSystem;
	ECS::DamageSystem damageSystem;
	ECS::DeathSystem deathSystem;
	ECS::PatrolSystem patrolSystem;
	ECS::EnemySystem enemySystem;
	ECS::TrunkSystem trunkSystem;
	ECS::ChickenSystem chickenSystem;
	ECS::GroundPatrolSystem groundPatrolSystem;
	ECS::EnemyDeathSystem enemyDeathSystem;
	ECS::PhysicsSystem physicsSystem;
	ECS::BoxSystem boxSystem;
	ECS::TrampolineSystem trampolineSystem;
	ECS::MovementSystem movementSystem;
	ECS::BulletSystem bulletSystem;
	ECS::PickupSystem pickupSystem;
	ECS::AnimationSystem animationSystem;
	ECS::PlayerAnimationSystem playerAnimationSystem;
	ECS::RenderSystem renderSystem;

	UI::Root hudInterface;
	UI::DataLoader hudLoader;

	Transition transition;

	std::string levelPath;
	int levelNumber = 1;
	float fallLimit = 0.0f;
	bool isRestarting = false;

	bool wasOnGround = false;
	bool deathSoundPlayed = false;
	float deathFlashTimer = 0.0f; // white "lightning" flash over the background on death
	float deathFallTimer = 0.0f;  // time spent tumbling after a damage death

	static constexpr float DEATH_FLASH_TIME = 0.2f; // duration of the death "lightning" flash
	static constexpr float DEATH_FALL_TIME = 0.5f;  // max tumble time before the restart kicks in

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

	// "Level X" banner: slides in from above the screen, holds, slides back out.
	enum class BannerPhase
	{
		Hidden,
		SlideIn,
		Hold,
		SlideOut,
		Done
	};

	BannerPhase bannerPhase = BannerPhase::Hidden;
	float bannerTimer = 0.0f;
	bool showLevelBanner = true; // false on checkpoint respawns

	static constexpr float BANNER_SLIDE_TIME = 0.45f; // slide in/out duration
	static constexpr float BANNER_HOLD_TIME = 2.0f;   // time fully visible
	static constexpr float BANNER_START_Y = -40.0f;   // off-screen above
	static constexpr float BANNER_TARGET_Y = 90.0f;   // about a third of the screen

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