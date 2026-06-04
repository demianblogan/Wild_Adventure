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
 "src/states/TestState.h" "src/states/TestState.cpp" "src/core/ResourceManager.h" "src/core/Resources.h" "src/core/ecs/Entity.h" "src/core/ecs/IComponentPool.h" "src/core/ecs/ComponentPool.h" "src/core/ecs/ComponentTypeId.h" "src/core/ecs/Registry.h" "src/components/Transform.h" "src/components/Sprite.h" "src/systems/RenderSystem.h" "src/systems/RenderSystem.cpp" "src/components/Velocity.h" "src/components/PreviousTransform.h" "src/systems/MovementSystem.h" "src/systems/MovementSystem.cpp" "src/components/Animation.h" "src/systems/AnimationSystem.h" "src/systems/AnimationSystem.cpp" "src/components/AnimationData.h" "src/components/AnimationSet.h" "src/components/AnimationState.h" "src/components/Facing.h" "src/core/VirtualScreen.h" "src/core/VirtualScreen.cpp" "src/core/DataLoader.h" "src/core/DataLoader.cpp" "src/ui/Element.h" "src/ui/Element.cpp"   "src/ui/Label.h" "src/ui/Label.cpp" "src/ui/Image.h" "src/ui/Image.cpp" "src/ui/InteractiveElement.h" "src/ui/InteractiveElement.cpp" "src/ui/Root.h" "src/ui/Root.cpp"   "src/ui/Button.h" "src/ui/Button.cpp" "src/ui/Checkbox.h" "src/ui/Checkbox.cpp" "src/ui/Slider.h" "src/ui/Slider.cpp" "src/ui/TextField.h" "src/ui/TextField.cpp" "src/ui/Animation.h" "src/ui/Animation.cpp" "src/ui/DataLoader.h" "src/ui/DataLoader.cpp" "src/audio/MusicPlayer.h" "src/audio/MusicPlayer.cpp" "src/audio/SoundPlayer.h" "src/audio/SoundPlayer.cpp" "src/audio/Mixer.h" "src/audio/Mixer.cpp" "src/tilemap/Tilemap.h" "src/tilemap/TilemapLoader.h" "src/tilemap/TilemapLoader.cpp" "src/tilemap/TilemapRenderer.h" "src/tilemap/TilemapRenderer.cpp" "src/graphics/AnimatedBackground.h" "src/graphics/AnimatedBackground.cpp" "src/core/Camera.h" "src/core/Camera.cpp" "src/components/Patrol.h" "src/systems/PatrolSystem.h" "src/systems/PatrolSystem.cpp" "src/states/SplashState.h" "src/states/SplashState.cpp" "src/core/Resources.cpp" "src/graphics/Transition.h" "src/graphics/Transition.cpp" "src/states/MenuBackdrop.h" "src/states/MenuBackdrop.cpp" "src/states/MenuState.h" "src/states/MenuState.cpp" "src/components/Collider.h" "src/components/Gravity.h" "src/components/Player.h" "src/components/CollisionState.h" "src/systems/InputSystem.h" "src/systems/InputSystem.cpp" "src/systems/PhysicsSystem.h" "src/systems/PhysicsSystem.cpp" "src/systems/PlayerAnimationSystem.h" "src/systems/PlayerAnimationSystem.cpp" "src/states/GameState.h" "src/states/GameState.cpp" "src/components/Jump.h" "src/systems/JumpSystem.h" "src/systems/JumpSystem.cpp")

target_link_libraries(Platformer PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    nlohmann_json::nlohmann_json
)

target_include_directories(Platformer PRIVATE src)
