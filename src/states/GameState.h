#pragma once

#include "core/Camera.h"
#include "core/DataLoader.h"
#include "core/State.h"
#include "core/ecs/Registry.h"
#include "graphics/ParticleSystem.h"
#include "graphics/ConfettiSystem.h"
#include "graphics/Transition.h"
#include "graphics/AnimatedBackground.h"
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