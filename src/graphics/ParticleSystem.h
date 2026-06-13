#pragma once

#include <SFML/System/Vector2.hpp>

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

struct Resources;
struct Tilemap;

namespace sf
{
	class RenderTarget;
}

class ParticleSystem
{
public:
	ParticleSystem(Resources& resources);

	void LoadConfig(const std::string& path);
	void SetTilemap(const Tilemap& tilemap);

	void Update(float deltaTime);
	void Draw(sf::RenderTarget& target);

	void Emit(const std::string& presetName, sf::Vector2f position, int directionX = 0);
	void EmitDebris(sf::Vector2f position, const std::string& textureName, int pieceCount, int directionX = 0);

	float GetRunBackOffset() const { return runBackOffset; }

private:
	enum class Kind { Dust, Debris };
	enum class DebrisPhase { Flying, Resting, Blinking };

	struct EmissionConfig
	{
		int count = 1;
		float speedX = 0.0f;
		float speedYMin = 0.0f;
		float speedYMax = 0.0f;
		float lifetime = 0.3f;
		float scale = 1.0f;
	};

	struct Particle
	{
		Kind kind = Kind::Dust;

		sf::Vector2f position;
		sf::Vector2f velocity;
		std::string texture;
		int frameIndex = 0;
		int frameCount = 1;
		float startScale = 1.0f;

		// Dust
		float age = 0.0f;
		float lifetime = 0.0f;

		// Debris
		DebrisPhase phase = DebrisPhase::Flying;
		float phaseTimer = 0.0f;
		float flyTimer = 0.0f;
		bool dead = false;
	};

	float RandomFloat(float min, float max);

	Resources& resources;
	const Tilemap* tilemap = nullptr;
	std::vector<Particle> particles;
	std::mt19937 randomEngine;

	std::unordered_map<std::string, EmissionConfig> presets;
	std::string textureName;
	float gravity = 0.0f;
	float runBackOffset = 0.0f;

	static constexpr float DEBRIS_GRAVITY = 600.0f;
	static constexpr float DEBRIS_SPREAD_X = 70.0f;
	static constexpr float DEBRIS_UP_MIN = 90.0f;
	static constexpr float DEBRIS_UP_MAX = 160.0f;
	static constexpr float DEBRIS_REST = 0.5f;
	static constexpr float DEBRIS_BLINK = 1.0f;
	static constexpr float DEBRIS_MAX_FLY = 2.0f;
};