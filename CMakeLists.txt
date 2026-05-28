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
 "src/states/TestState.h" "src/states/TestState.cpp" "src/core/ResourceManager.h" "src/core/Resources.h" "src/core/ecs/Entity.h" "src/core/ecs/IComponentPool.h" "src/core/ecs/ComponentPool.h" "src/core/ecs/ComponentTypeId.h" "src/core/ecs/Registry.h" "src/components/Transform.h" "src/components/Sprite.h" "src/systems/RenderSystem.h" "src/systems/RenderSystem.cpp" "src/components/Velocity.h" "src/components/PreviousTransform.h" "src/systems/MovementSystem.h" "src/systems/MovementSystem.cpp" "src/components/Animation.h" "src/systems/AnimationSystem.h" "src/systems/AnimationSystem.cpp" "src/components/AnimationData.h" "src/components/AnimationSet.h" "src/components/AnimationState.h" "src/components/Facing.h" "src/core/VirtualScreen.h" "src/core/VirtualScreen.cpp" "src/core/DataLoader.h" "src/core/DataLoader.cpp" "src/ui/Element.h" "src/ui/Element.cpp"   "src/ui/Label.h" "src/ui/Label.cpp" "src/ui/Image.h" "src/ui/Image.cpp" "src/ui/InteractiveElement.h" "src/ui/InteractiveElement.cpp" "src/ui/Root.h" "src/ui/Root.cpp"   "src/ui/Button.h" "src/ui/Button.cpp")

target_link_libraries(Platformer PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    nlohmann_json::nlohmann_json
)

target_include_directories(Platformer PRIVATE src)
