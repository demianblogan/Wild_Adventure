#include "ParticleSystem.h"

#include "core/Resources.h"

#include <nlohmann/json.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <stdexcept>

ParticleSystem::ParticleSystem(Resources& resources)
	: resources(resources)
	, randomEngine(std::random_device{}())
{}

void ParticleSystem::LoadConfig(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("ParticleSystem: cannot open config: " + path);

	const nlohmann::json data = nlohmann::json::parse(file);

	textureName = data.at("texture");
	gravity = data.value("gravity", 0.0f);
	runBackOffset = data.value("runBackOffset", 0.0f);

	presets.clear();
	for (const auto& [name, presetData] : data.at("presets").items())
	{
		EmissionConfig config;
		config.count = presetData.at("count");
		config.speedX = presetData.at("speedX");
		config.speedYMin = presetData.at("speedYMin");
		config.speedYMax = presetData.at("speedYMax");
		config.lifetime = presetData.at("lifetime");
		config.scale = presetData.at("scale");
		presets[name] = config;
	}
}

float ParticleSystem::RandomFloat(float min, float max)
{
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine);
}

void ParticleSystem::Emit(const std::string& presetName, sf::Vector2f position, int directionX)
{
	const auto found = presets.find(presetName);
	if (found == presets.end())
		return;

	const EmissionConfig& config = found->second;

	for (int i = 0; i < config.count; i++)
	{
		Particle particle;
		particle.position = position;

		if (directionX == 0)
			particle.velocity.x = RandomFloat(-config.speedX, config.speedX);
		else
			particle.velocity.x = directionX * RandomFloat(config.speedX * 0.4f, config.speedX);

		particle.velocity.y = RandomFloat(config.speedYMin, config.speedYMax);
		particle.age = 0.0f;
		particle.lifetime = config.lifetime * RandomFloat(0.8f, 1.2f);
		particle.startScale = config.scale * RandomFloat(0.7f, 1.1f);

		particles.push_back(particle);
	}
}

void ParticleSystem::Update(float deltaTime)
{
	for (Particle& particle : particles)
	{
		particle.velocity.y += gravity * deltaTime;
		particle.position += particle.velocity * deltaTime;
		particle.age += deltaTime;
	}

	std::erase_if(particles, [](const Particle& particle) { return particle.age >= particle.lifetime; });
}

void ParticleSystem::Draw(sf::RenderTarget& target)
{
	if (particles.empty())
		return;

	const sf::Texture& texture = resources.textures.Get(textureName);
	const sf::Vector2f textureSize(texture.getSize());

	sf::Sprite sprite(texture);
	sprite.setOrigin(textureSize / 2.0f);

	for (const Particle& particle : particles)
	{
		const float progress = (particle.lifetime > 0.0f) ? (particle.age / particle.lifetime) : 1.0f;
		const float remaining = std::clamp(1.0f - progress, 0.0f, 1.0f);

		const float scale = particle.startScale * remaining;
		const auto alpha = static_cast<std::uint8_t>(remaining * 255.0f);

		sprite.setPosition({ std::floor(particle.position.x), std::floor(particle.position.y) });
		sprite.setScale({ scale, scale });
		sprite.setColor(sf::Color(255, 255, 255, alpha));

		target.draw(sprite);
	}
}