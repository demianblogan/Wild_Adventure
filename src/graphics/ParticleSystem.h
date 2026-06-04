#pragma once

#include <SFML/System/Vector2.hpp>

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

struct Resources;

namespace sf
{
	class RenderTarget;
}

class ParticleSystem
{
public:
	ParticleSystem(Resources& resources);

	void LoadConfig(const std::string& path);

	void Update(float deltaTime);
	void Draw(sf::RenderTarget& target);

	// directionX: 0 = symmetric spread; -1/+1 = biased to one side (wall jump).
	void Emit(const std::string& presetName, sf::Vector2f position, int directionX = 0);

	float GetRunBackOffset() const { return runBackOffset; }

private:
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
		sf::Vector2f position;
		sf::Vector2f velocity;
		float age = 0.0f;
		float lifetime = 0.0f;
		float startScale = 1.0f;
	};

	float RandomFloat(float min, float max);

	Resources& resources;
	std::vector<Particle> particles;
	std::mt19937 randomEngine;

	std::unordered_map<std::string, EmissionConfig> presets;
	std::string textureName;
	float gravity = 0.0f;

	float runBackOffset = 0.0f;
};