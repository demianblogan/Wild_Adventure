#include "LevelSetup.h"

#include "components/render/Animation.h"
#include "components/render/AnimationSet.h"
#include "components/render/Sprite.h"
#include "core/Resources.h"
#include "core/ecs/Registry.h"
#include "graphics/AnimatedBackground.h"

#include <SFML/Graphics/Color.hpp>

#include <string>

namespace LevelSetup
{
	// Each level has its own music track, listed in data/levels/music.json.
	// Levels past the last track cycle through the pool again.
	std::string MusicTrack(Resources& resources, int levelNumber)
	{
		const nlohmann::json* data = resources.TryGetJSON("data/levels/music.json");

		if (data != nullptr && data->contains("tracks"))
		{
			const nlohmann::json& tracks = data->at("tracks");
			if (!tracks.empty())
			{
				int index = (levelNumber - 1) % static_cast<int>(tracks.size());
				if (index < 0)
					index = 0;

				return tracks.at(index).get<std::string>();
			}
		}

		return "background1"; // fallback when the config is missing or empty
	}

	// The level's visual theme ("green", "hot", "sweet", ...) is the map's
	// Class field in Tiled; it picks the color grading palette. Tiled omits
	// the key entirely while the class is unset.
	std::string Theme(const nlohmann::json& mapJSON)
	{
		const std::string theme = mapJSON.value("class", "");
		return theme.empty() ? "default" : theme;
	}

	// Per-level animated background: texture, scroll direction and speed come
	// from data/levels/backgrounds.json, keyed by level number, with a
	// "default" entry for levels that have no override.
	void ApplyBackground(Resources& resources, AnimatedBackground& background, int levelNumber)
	{
		std::string texture = "blue";
		std::string directionName = "right";
		float speed = 16.0f;

		if (const nlohmann::json* data = resources.TryGetJSON("data/levels/backgrounds.json"))
		{
			const std::string key = std::to_string(levelNumber);

			const nlohmann::json* entry = nullptr;
			if (data->contains(key))
				entry = &data->at(key);
			else if (data->contains("default"))
				entry = &data->at("default");

			if (entry != nullptr)
			{
				texture = entry->value("texture", texture);
				directionName = entry->value("direction", directionName);
				speed = entry->value("speed", speed);

				// Optional [r, g, b] multiplier, 0-255: darkens cave levels.
				if (entry->contains("tint"))
				{
					const nlohmann::json& tint = entry->at("tint");
					background.SetTint(sf::Color(tint.at(0), tint.at(1), tint.at(2)));
				}
			}
		}

		AnimatedBackground::Direction direction = AnimatedBackground::Direction::Right;
		if (directionName == "up")
			direction = AnimatedBackground::Direction::Up;
		else if (directionName == "down")
			direction = AnimatedBackground::Direction::Down;
		else if (directionName == "left")
			direction = AnimatedBackground::Direction::Left;

		background.SetTexture(texture);
		background.SetDirection(direction);
		background.SetSpeed(speed);
	}

	// Per-level "lamp" lighting: outside a soft circle around the player the
	// world is dark. Configured in data/levels/lighting.json by level number;
	// levels without an entry stay fully lit.
	LevelLighting LoadLighting(Resources& resources, int levelNumber)
	{
		LevelLighting lighting;

		const nlohmann::json* data = resources.TryGetJSON("data/levels/lighting.json");
		if (data == nullptr)
			return lighting;

		const std::string key = std::to_string(levelNumber);
		if (!data->contains(key))
			return lighting;

		const nlohmann::json& entry = data->at(key);
		lighting.enabled  = true;
		lighting.radius   = entry.value("radius", lighting.radius);
		lighting.darkness = entry.value("darkness", lighting.darkness);

		return lighting;
	}

	// The player prefab is authored with the default ninja_frog_* textures;
	// the other skins reuse identical sheets under their own prefix, so
	// switching is a texture-name rewrite on the freshly spawned player.
	void ApplySkin(ECS::Registry& registry, ECS::Entity player, const std::string& skinId)
	{
		const std::string defaultPrefix = "ninja_frog";

		if (skinId.empty() || skinId == defaultPrefix)
			return;

		const auto reskin = [&](std::string& textureName)
		{
			if (textureName.rfind(defaultPrefix, 0) == 0)
				textureName = skinId + textureName.substr(defaultPrefix.size());
		};

		if (registry.Has<ECS::Sprite>(player))
			reskin(registry.Get<ECS::Sprite>(player).textureName);

		if (registry.Has<ECS::AnimationSet>(player))
		{
			for (auto& [state, animation] : registry.Get<ECS::AnimationSet>(player).animations)
				reskin(animation.textureName);
		}

		// The Animation component already holds a copy of the current state's
		// data, made when the prefab was loaded.
		if (registry.Has<ECS::Animation>(player))
			reskin(registry.Get<ECS::Animation>(player).data.textureName);
	}
}
