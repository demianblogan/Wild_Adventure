#pragma once

#include "core/ecs/Entity.h"

#include <nlohmann/json.hpp>

#include <string>

class AnimatedBackground;
struct Resources;

namespace ECS
{
	class Registry;
}

// Lighting applied to cave levels: the player sees a soft-edged lamp circle;
// everything outside is dark. Loaded from data/levels/lighting.json.
struct LevelLighting
{
	bool enabled = false;
	float radius = 90.0f;   // fully dark at this distance from the player, in pixels
	float darkness = 0.95f; // shade outside the circle, 0..1 (1 = pure black)
};

namespace LevelSetup
{
	// Music track name for a given level number, from data/levels/music.json
	// (cycles through the track pool when past the last entry).
	std::string MusicTrack(Resources& resources, int levelNumber);

	// The visual theme string from the map's Class field in Tiled ("green", "water", …).
	// Returns "default" when the field is absent.
	std::string Theme(const nlohmann::json& mapJSON);

	// Configures background texture, scroll direction and speed from data/levels/backgrounds.json.
	void ApplyBackground(Resources& resources, AnimatedBackground& background, int levelNumber);

	// Loads per-level lamp-lighting settings from data/levels/lighting.json.
	LevelLighting LoadLighting(Resources& resources, int levelNumber);

	// Rewrites ninja_frog_* texture names on the freshly spawned player to match
	// the chosen skin (a no-op when the default skin is selected).
	void ApplySkin(ECS::Registry& registry, ECS::Entity player, const std::string& skinId);
}
