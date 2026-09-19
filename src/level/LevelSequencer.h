#pragma once

#include "core/ecs/Entity.h"
#include "level/ProgressSnapshot.h"

#include <SFML/System/Vector2.hpp>

#include <optional>

class Camera;
class ConfettiSystem;
class HUD;
class SceneLoader;
class Transition;

namespace Audio
{
	class Mixer;
}

namespace ECS
{
	class Registry;
}

// Drives a level from its opening wipe through to completion: the appear/
// disappear effect entities, the start-platform and finish-cup touch
// reactions, and the checkpoint system. Pulled out of GameState so that class
// is left with orchestration (which systems run, in what order) rather than
// also owning this state machine.
class LevelSequencer
{
public:
	enum class Phase
	{
		Revealing,
		Appearing,
		Playing,
		Finishing,
		Disappearing,
		Complete
	};

	LevelSequencer(ECS::Registry& registry, SceneLoader& sceneLoader, Camera& camera,
		ConfettiSystem& confetti, Audio::Mixer& audioMixer, Transition& transition, HUD& hud);

	// The scene loader hands these back after populating the level; call once
	// right after the scene (and, for the player, the player prefab) loads.
	void SetPlayer(ECS::Entity entity) { playerEntity = entity; }
	void SetStartPlatform(ECS::Entity entity) { startPlatformEntity = entity; }
	void SetFinish(ECS::Entity entity) { finishEntity = entity; }

	// SpawnPlayer computes the true respawn point (start platform or a
	// checkpoint override); this just records it for a later restart.
	void SetRespawnPoint(sf::Vector2f point) { respawnPoint = point; }

	// Advances the phase state machine and, while Playing, the checkpoint
	// checks. `score`/`fruitsCollected`/`enemiesKilled` are frozen into the
	// checkpoint snapshot at the instant a checkpoint is touched; the caller
	// still owns and updates the real counters.
	// Returns true on the single frame the level transitions into Complete,
	// so the caller can push LevelCompleteState with whatever stats it has.
	bool Update(float deltaTime, int score, int fruitsCollected, int enemiesKilled);

	Phase GetPhase() const { return phase; }

	sf::Vector2f GetRespawnPoint() const { return respawnPoint; }
	int GetCheckpointScore() const { return checkpointScore; }
	int GetCheckpointFruitsCollected() const { return checkpointFruitsCollected; }
	int GetCheckpointEnemiesKilled() const { return checkpointEnemiesKilled; }
	const std::optional<ProgressSnapshot>& GetCheckpointSnapshot() const { return checkpointSnapshot; }

private:
	void UpdateCheckpoints(int score, int fruitsCollected, int enemiesKilled);

	bool IsPlayerOnFinish() const;
	bool IsPlayerOnStartPlatform() const;

	ECS::Registry& registry;
	SceneLoader& sceneLoader;
	Camera& camera;
	ConfettiSystem& confetti;
	Audio::Mixer& audioMixer;
	Transition& transition;
	HUD& hud;

	Phase phase = Phase::Revealing;

	ECS::Entity playerEntity = ECS::InvalidEntity;
	ECS::Entity startPlatformEntity = ECS::InvalidEntity;
	ECS::Entity finishEntity = ECS::InvalidEntity;
	ECS::Entity appearEffectEntity = ECS::InvalidEntity;
	ECS::Entity disappearEffectEntity = ECS::InvalidEntity;

	bool hasPlayedStartMoving = false;
	float finishTimer = 0.0f;
	bool hasShownLevelComplete = false;

	sf::Vector2f respawnPoint;
	int checkpointScore = 0;
	int checkpointFruitsCollected = 0;
	int checkpointEnemiesKilled = 0;
	std::optional<ProgressSnapshot> checkpointSnapshot;

	// Camera shake trauma and confetti rise for a friendly touch (start
	// platform, checkpoint, finish) -- gentler than a combat hit.
	static constexpr float ShakeTouch = 0.45f;
	static constexpr float ConfettiRise = 40.0f; // pixels above the touched object's base

	static constexpr float FinishRiseTime = 0.3f; // bounce arc before the hero vanishes
};
