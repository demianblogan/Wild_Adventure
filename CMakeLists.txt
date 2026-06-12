cmake_minimum_required(VERSION 3.21)

project(Platformer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 3 COMPONENTS Graphics Window System Audio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(Platformer
    src/main.cpp
    src/Application.h
    src/Application.cpp
	src/Context.h
	src/core/State.h
	src/core/StateMachine.h
	src/core/StateMachine.cpp
 "src/states/TestState.h" "src/states/TestState.cpp" "src/core/ResourceManager.h" "src/core/Resources.h" "src/core/ecs/Entity.h" "src/core/ecs/IComponentPool.h" "src/core/ecs/ComponentPool.h" "src/core/ecs/ComponentTypeId.h" "src/core/ecs/Registry.h" "src/components/Transform.h" "src/components/Sprite.h" "src/systems/RenderSystem.h" "src/systems/RenderSystem.cpp" "src/components/Velocity.h" "src/components/PreviousTransform.h" "src/systems/MovementSystem.h" "src/systems/MovementSystem.cpp" "src/components/Animation.h" "src/systems/AnimationSystem.h" "src/systems/AnimationSystem.cpp" "src/components/AnimationData.h" "src/components/AnimationSet.h" "src/components/AnimationState.h" "src/components/Facing.h" "src/core/VirtualScreen.h" "src/core/VirtualScreen.cpp" "src/core/DataLoader.h" "src/core/DataLoader.cpp" "src/ui/Element.h" "src/ui/Element.cpp"   "src/ui/Label.h" "src/ui/Label.cpp" "src/ui/Image.h" "src/ui/Image.cpp" "src/ui/InteractiveElement.h" "src/ui/InteractiveElement.cpp" "src/ui/Root.h" "src/ui/Root.cpp"   "src/ui/Button.h" "src/ui/Button.cpp" "src/ui/Checkbox.h" "src/ui/Checkbox.cpp" "src/ui/Slider.h" "src/ui/Slider.cpp" "src/ui/TextField.h" "src/ui/TextField.cpp" "src/ui/Animation.h" "src/ui/Animation.cpp" "src/ui/DataLoader.h" "src/ui/DataLoader.cpp" "src/audio/MusicPlayer.h" "src/audio/MusicPlayer.cpp" "src/audio/SoundPlayer.h" "src/audio/SoundPlayer.cpp" "src/audio/Mixer.h" "src/audio/Mixer.cpp" "src/tilemap/Tilemap.h" "src/tilemap/TilemapLoader.h" "src/tilemap/TilemapLoader.cpp" "src/tilemap/TilemapRenderer.h" "src/tilemap/TilemapRenderer.cpp" "src/graphics/AnimatedBackground.h" "src/graphics/AnimatedBackground.cpp" "src/core/Camera.h" "src/core/Camera.cpp" "src/components/Patrol.h" "src/systems/PatrolSystem.h" "src/systems/PatrolSystem.cpp" "src/states/SplashState.h" "src/states/SplashState.cpp" "src/core/Resources.cpp" "src/graphics/Transition.h" "src/graphics/Transition.cpp" "src/states/MenuBackdrop.h" "src/states/MenuBackdrop.cpp" "src/states/MenuState.h" "src/states/MenuState.cpp" "src/components/Collider.h" "src/components/Gravity.h" "src/components/Player.h" "src/components/CollisionState.h" "src/systems/InputSystem.h" "src/systems/InputSystem.cpp" "src/systems/PhysicsSystem.h" "src/systems/PhysicsSystem.cpp" "src/systems/PlayerAnimationSystem.h" "src/systems/PlayerAnimationSystem.cpp" "src/states/GameState.h" "src/states/GameState.cpp" "src/components/Jump.h" "src/systems/JumpSystem.h" "src/systems/JumpSystem.cpp" "src/components/WallSlide.h"   "src/components/Health.h" "src/components/Hazard.h" "src/components/Hitbox.h" "src/systems/DamageSystem.h" "src/systems/DamageSystem.cpp" "src/components/Rotation.h" "src/systems/DeathSystem.h" "src/systems/DeathSystem.cpp" "src/components/Collectible.h" "src/components/Despawning.h" "src/systems/PickupSystem.h" "src/systems/PickupSystem.cpp" "src/components/Solid.h" "src/components/Box.h" "src/components/BoxHit.h" "src/systems/BoxSystem.h" "src/systems/BoxSystem.cpp" "src/components/PickupDelay.h" "src/graphics/ParticleSystem.h" "src/graphics/ParticleSystem.h" "src/graphics/ParticleSystem.cpp" "src/core/Input.h" "src/core/Input.cpp" "src/core/Settings.h" "src/core/Settings.cpp" "src/states/ConfirmState.h" "src/states/ConfirmState.cpp" "src/core/GraphicsTarget.h" "src/ui/Stepper.h" "src/ui/Stepper.cpp"  "src/states/CompanySplashState.h" "src/states/CompanySplashState.cpp" "src/components/Finish.h" "src/graphics/ConfettiSystem.h" "src/graphics/ConfettiSystem.cpp" "src/states/SettingsController.h" "src/states/SettingsController.cpp" "src/states/PauseState.h" "src/states/PauseState.cpp" "src/states/LevelCompleteState.h" "src/states/LevelCompleteState.cpp" "src/components/Enemy.h" "src/components/GroundPatrol.h" "src/components/EnemyDeath.h" "src/components/TrunkAI.h" "src/components/Bullet.h" "src/components/BulletPiece.h" "src/systems/EnemySystem.h" "src/systems/EnemySystem.cpp" "src/systems/GroundPatrolSystem.h" "src/systems/GroundPatrolSystem.cpp" "src/systems/EnemyDeathSystem.h" "src/systems/EnemyDeathSystem.cpp" "src/systems/TrunkSystem.h" "src/systems/TrunkSystem.cpp" "src/systems/BulletSystem.h" "src/systems/BulletSystem.cpp" "src/components/Trampoline.h" "src/systems/TrampolineSystem.h" "src/systems/TrampolineSystem.cpp" "src/core/Campaign.h" "src/core/Campaign.cpp" "src/states/SelectLevelController.h" "src/states/SelectLevelController.cpp" "src/states/CampaignVictoryState.h" "src/states/CampaignVictoryState.cpp" "src/graphics/ColorGrading.h" "src/graphics/ColorGrading.cpp" "src/components/ChickenAI.h" "src/systems/ChickenSystem.h" "src/systems/ChickenSystem.cpp" "src/graphics/LightOverlay.h" "src/graphics/LightOverlay.cpp")

target_link_libraries(Platformer PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    nlohmann_json::nlohmann_json
)

target_include_directories(Platformer PRIVATE src)
