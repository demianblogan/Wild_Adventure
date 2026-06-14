# 🦊 Wild Adventure

A handcrafted pixel-art 2D platformer built with **C++23**, **SFML 3**, and a fully custom **Entity-Component-System architecture**.

Run, jump, explore, defeat enemies, avoid deadly traps, collect fruits, and master every challenge to achieve 100% completion.

---

## 🎮 Gameplay

https://github.com/user-attachments/assets/86ca9e37-d763-4c70-8055-ace9462b6b93

---

## ✨ Features

### 🎯 Gameplay

* 9 handcrafted platforming levels
* 9 unique enemy types with distinct behaviors
* Flying and ground-based enemies
* Checkpoint system
* Collectible fruits
* Destructible fruit crates
* Trampolines and air jump boosters
* Multiple trap types including spikes and fire hazards
* Three-star level rating system
* Level completion statistics
* 100% completion tracking
* Campaign progression system
* Unlockable character skins
* Rewards for perfect level completion
* End-of-campaign victory scene

### 🎮 Input Support

* Full keyboard support
* Xbox controller support
* PlayStation controller support
* Controller-friendly menus
* Full menu navigation without a mouse
* Rebindable controls

### 🎨 Visuals & Audio

* Pixel-art graphics
* Animated characters and enemies
* Particle effects
* Smooth camera system
* Dynamic sound effects
* Background music system
* Scene transition effects

### ⚙️ Settings

The game includes a configurable settings system accessible from both the Main Menu and Pause Menu.

Available options:

* Music volume
* Sound effects volume
* Graphics settings
* Fullscreen mode
* Keyboard controls
* Controller controls

Settings are automatically saved and restored between sessions.

---

## 🕹 Controls

| Action      | Keyboard       | Xbox               | PlayStation        |
| ----------- | -------------- | ------------------ | ------------------ |
| Move        | A / D or ← / → | Left Stick / D-Pad | Left Stick / D-Pad |
| Jump        | Space          | A                  | Cross (✕)          |
| Confirm     | Enter          | A                  | Cross (✕)          |
| Back        | Escape         | B                  | Circle (◯)         |
| Pause       | Escape         | Menu               | Options            |
| Navigate UI | Arrow Keys     | D-Pad / Left Stick | D-Pad / Left Stick |

---

## 🏗 Technical Highlights

Wild Adventure was developed with a strong focus on architecture, maintainability, and scalable game systems.

### Core Architecture

* Custom Sparse-Set Entity Component System (ECS)
* State Machine architecture
* Data-driven game design
* Resource management system
* Audio management system
* Save/load system
* Event-driven gameplay systems

### Engine Systems

* Physics system
* Animation system
* Camera system
* Rendering system
* Particle system
* Collision handling
* Enemy AI systems
* Trap systems
* Custom UI framework

### Content Pipeline

* Tiled map integration
* JSON-based configuration
* JSON-defined UI layouts
* Data-driven entity definitions
* Data-driven gameplay parameters

---

## 🧠 Data-Driven Design

Most game content is configured through external JSON files rather than hardcoded values.

This includes:

* Player configuration
* Enemy configuration
* UI layouts
* Audio definitions
* Game settings
* Particle effects
* Campaign progression data
* Entity prefabs

This allows gameplay balancing and content iteration without recompiling the project.

---

## 📂 Project Structure

```text
assets/     → textures, sounds, music, fonts
data/       → game data, UI layouts, configuration
src/        → source code

CMakeLists.txt
vcpkg.json
README.md
```

## ⚙️ Technologies

* C++23
* SFML 3
* nlohmann/json
* CMake
* vcpkg
* Tiled

---

## 📦 Download

👉 Download the latest release from the Releases page.

Quick start:

1. Download the latest release archive
2. Extract it
3. Run `WildAdventure.exe`

---

## 🚀 Building

### Requirements

* C++23 compatible compiler
* CMake 3.25+
* vcpkg

### Configure

```bash
cmake --preset default
```

### Build

```bash
cmake --build build
```

### Run

Launch the game from the project root directory so all assets can be loaded correctly.

---

## 📚 What I Learned

This project was created as a large-scale practice project focused on game architecture and gameplay programming.

Key areas explored during development:

* Entity Component System design
* Data-driven architecture
* UI framework development
* Resource management
* State machines
* Serialization
* Tiled integration
* Save systems
* Large project organization in modern C++

---

## 📄 License

Released under the MIT License.
