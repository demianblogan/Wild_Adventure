#pragma once

#include "core/Camera.h"
#include "core/DataLoader.h"
#include "core/State.h"
#include "core/ecs/Registry.h"
#include "graphics/ParticleSystem.h"
#include "graphics/Transition.h"
#include "systems/AnimationSystem.h"
#include "systems/InputSystem.h"
#include "systems/JumpSystem.h"
#include "systems/MovementSystem.h"
#include "systems/PatrolSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/PlayerAnimationSystem.h"
#include "systems/RenderSystem.h"
#include "tilemap/Tilemap.h"

#include <string>

class GameState : public State
{
public:
	GameState(Context& context, const std::string& levelPath);

	void HandleEvent(const sf::Event& event) override;
	void Update(float deltaTime) override;
	void Render(float interpolationFactor) override;

private:
	ECS::Registry registry;
	DataLoader sceneLoader;

	Camera camera;
	Tilemap tilemap;

	ECS::InputSystem inputSystem;
	ECS::JumpSystem jumpSystem;
	ECS::PatrolSystem patrolSystem;
	ECS::PhysicsSystem physicsSystem;
	ECS::MovementSystem movementSystem;
	ECS::AnimationSystem animationSystem;
	ECS::PlayerAnimationSystem playerAnimationSystem;
	ECS::RenderSystem renderSystem;

	ParticleSystem particles;
	Transition transition;

	bool wasOnGround = false;
	int previousJumpsRemaining = 0;
	float runDustTimer = 0.0f;
	float previousLockTimer = 0.0f;

	static constexpr float RUN_DUST_INTERVAL = 0.12f;
};