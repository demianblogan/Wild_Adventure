#include "ParticleSystem.h"

#include "core/Random.h"
#include "core/Resources.h"
#include "tilemap/Tilemap.h"

#include <nlohmann/json.hpp>

#include <SFML/Graphics/CircleShape.hpp>
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
{}

void ParticleSystem::SetTilemap(const Tilemap& tilemap)
{
	this->tilemap = &tilemap;
}

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

void ParticleSystem::Emit(const std::string& presetName, sf::Vector2f position, int directionX)
{
	const auto found = presets.find(presetName);
	if (found == presets.end())
		return;

	const EmissionConfig& config = found->second;

	for (int i = 0; i < config.count; i++)
	{
		Particle particle;
		particle.kind = Kind::Dust;
		particle.texture = textureName;
		particle.frameCount = 1;
		particle.frameIndex = 0;
		particle.position = position;

		if (directionX == 0)
			particle.velocity.x = Random::Float(-config.speedX, config.speedX);
		else
			particle.velocity.x = directionX * Random::Float(config.speedX * 0.4f, config.speedX);

		particle.velocity.y = Random::Float(config.speedYMin, config.speedYMax);
		particle.age = 0.0f;
		particle.lifetime = config.lifetime * Random::Float(0.8f, 1.2f);
		particle.startScale = config.scale * Random::Float(0.7f, 1.1f);

		particles.push_back(particle);
	}
}

void ParticleSystem::EmitRunDust(sf::Vector2f feet, int direction)
{
	Emit("run", { feet.x - direction * runBackOffset, feet.y });
}

void ParticleSystem::EmitDebris(sf::Vector2f position, const std::string& textureName, int pieceCount, int directionX)
{
	for (int i = 0; i < pieceCount; i++)
	{
		Particle particle;
		particle.kind = Kind::Debris;
		particle.texture = textureName;
		particle.frameCount = pieceCount;
		particle.frameIndex = i;
		particle.position = position;

		// directionX == 0 spreads pieces both ways (box debris); a non-zero value
		// biases them to fly off in that direction (bullet pieces bouncing off a wall).
		const float velocityX = (directionX == 0)
			? Random::Float(-DEBRIS_SPREAD_X, DEBRIS_SPREAD_X)
			: directionX * Random::Float(DEBRIS_SPREAD_X * 0.4f, DEBRIS_SPREAD_X);

		particle.velocity = { velocityX, -Random::Float(DEBRIS_UP_MIN, DEBRIS_UP_MAX) };
		particle.startScale = 1.0f;
		particle.phase = DebrisPhase::Flying;

		particles.push_back(particle);
	}
}

void ParticleSystem::EmitGhostTrail(sf::Vector2f position, int facingDir)
{
	Particle particle;
	particle.kind = Kind::Dust;
	particle.texture = "ghost_particles";
	particle.frameCount = 4;
	particle.animated = true;

	// Spawn a touch off the back at a varied height, then drift straight out (no gravity).
	particle.position = { position.x + Random::Float(-2.0f, 2.0f), position.y + Random::Float(-9.0f, 9.0f) };
	particle.velocity = { static_cast<float>(-facingDir) * Random::Float(10.0f, 28.0f), 0.0f };
	particle.age = 0.0f;
	particle.lifetime = Random::Float(0.3f, 0.5f);
	particle.startScale = Random::Float(0.8f, 1.1f);

	particles.push_back(particle);
}

void ParticleSystem::EmitBubble(sf::Vector2f position)
{
	Particle particle;
	particle.kind = Kind::Bubble;
	particle.position = position;
	particle.velocity = { Random::Float(-5.0f, 5.0f), -Random::Float(25.0f, 55.0f) };
	particle.age = 0.0f;
	particle.lifetime = Random::Float(2.5f, 4.5f);
	particle.startScale = Random::Float(0.9f, 2.0f); // radius in virtual pixels
	particles.push_back(particle);
}

void ParticleSystem::Update(float deltaTime)
{
	const float tileSize = (tilemap != nullptr) ? static_cast<float>(tilemap->tileSize) : 16.0f;

	for (Particle& particle : particles)
	{
		if (particle.kind == Kind::Dust)
		{
			// Animated wisps (the ghost's) drift straight; plain dust is pulled by gravity.
			if (!particle.animated)
				particle.velocity.y += gravity * deltaTime;
			particle.position += particle.velocity * deltaTime;
			particle.age += deltaTime;
			continue;
		}

		if (particle.kind == Kind::Bubble)
		{
			// Rises (no gravity), wobbling sideways as it goes.
			particle.position.x += (particle.velocity.x + std::sin(particle.age * 3.5f) * 10.0f) * deltaTime;
			particle.position.y += particle.velocity.y * deltaTime;
			particle.age += deltaTime;
			continue;
		}

		// Debris.
		if (particle.phase == DebrisPhase::Flying)
		{
			particle.velocity.y += DEBRIS_GRAVITY * deltaTime;
			particle.position += particle.velocity * deltaTime;
			particle.flyTimer += deltaTime;

			bool landed = false;
			if (tilemap != nullptr)
			{
				const int column = static_cast<int>(std::floor(particle.position.x / tileSize));
				const int row = static_cast<int>(std::floor(particle.position.y / tileSize));

				const bool inSolid = tilemap->IsSolid(column, row);
				const bool solidAbove = tilemap->IsSolid(column, row - 1);

				if (inSolid && !solidAbove && particle.velocity.y >= 0.0f)
				{
					// Resting on a top surface (floor or ledge), not a wall side.
					particle.position.y = row * tileSize;
					particle.velocity = { 0.0f, 0.0f };
					particle.phase = DebrisPhase::Resting;
					particle.phaseTimer = DEBRIS_REST;
					landed = true;
				}
				else if (inSolid)
				{
					// Hit a wall from the side: push back out and keep falling along it.
					if (particle.velocity.x > 0.0f)
						particle.position.x = column * tileSize - 0.001f;
					else if (particle.velocity.x < 0.0f)
						particle.position.x = (column + 1) * tileSize + 0.001f;

					particle.velocity.x = 0.0f;
				}
			}

			if (!landed && particle.flyTimer >= DEBRIS_MAX_FLY)
				particle.dead = true; // fell into a pit / never landed
		}
		else if (particle.phase == DebrisPhase::Resting)
		{
			particle.phaseTimer -= deltaTime;
			if (particle.phaseTimer <= 0.0f)
			{
				particle.phase = DebrisPhase::Blinking;
				particle.phaseTimer = DEBRIS_BLINK;
			}
		}
		else // Blinking
		{
			particle.phaseTimer -= deltaTime;
			if (particle.phaseTimer <= 0.0f)
				particle.dead = true;
		}
	}

	std::erase_if(particles, [](const Particle& particle)
		{
			if (particle.kind == Kind::Debris)
				return particle.dead;
			return particle.age >= particle.lifetime; // Dust and Bubble
		});
}

void ParticleSystem::Draw(sf::RenderTarget& target)
{
	for (const Particle& particle : particles)
	{
		// Bubbles are drawn procedurally (a faint ring), not from a texture.
		if (particle.kind == Kind::Bubble)
		{
			const float progress  = (particle.lifetime > 0.0f) ? (particle.age / particle.lifetime) : 1.0f;
			const float remaining = std::clamp(1.0f - progress, 0.0f, 1.0f);
			const float radius    = particle.startScale;
			const auto  alpha     = static_cast<std::uint8_t>(remaining * 150.0f);

			sf::CircleShape bubble(radius);
			bubble.setOrigin({ radius, radius });
			bubble.setPosition({ std::floor(particle.position.x), std::floor(particle.position.y) });
			bubble.setFillColor(sf::Color(180, 220, 255, static_cast<std::uint8_t>(alpha / 3)));
			bubble.setOutlineThickness(0.5f);
			bubble.setOutlineColor(sf::Color(215, 238, 255, alpha));
			target.draw(bubble);
			continue;
		}

		if (particle.kind == Kind::Debris && particle.phase == DebrisPhase::Blinking)
		{
			if (std::fmod(particle.phaseTimer, 0.16f) < 0.08f) // blink off
				continue;
		}

		const sf::Texture& texture = resources.textures.Get(particle.texture);
		const int pieceWidth = static_cast<int>(texture.getSize().x) / particle.frameCount;
		const int pieceHeight = static_cast<int>(texture.getSize().y);

		const float dustProgress = (particle.lifetime > 0.0f) ? (particle.age / particle.lifetime) : 1.0f;

		// Animated particles cycle through their frames over their life; others hold one frame.
		int displayedFrame = particle.frameIndex;
		if (particle.animated)
			displayedFrame = std::clamp(static_cast<int>(dustProgress * particle.frameCount), 0, particle.frameCount - 1);

		sf::Sprite sprite(texture);
		sprite.setTextureRect(sf::IntRect({ displayedFrame * pieceWidth, 0 }, { pieceWidth, pieceHeight }));
		sprite.setOrigin({ pieceWidth / 2.0f, pieceHeight / 2.0f });

		if (particle.kind == Kind::Dust)
		{
			const float remaining = std::clamp(1.0f - dustProgress, 0.0f, 1.0f);
			const auto alpha = static_cast<std::uint8_t>(remaining * 255.0f);

			// Animated wisps keep their size and just fade; plain dust shrinks as it fades.
			const float scale = particle.animated ? particle.startScale : particle.startScale * remaining;
			sprite.setScale({ scale, scale });
			sprite.setColor(sf::Color(255, 255, 255, alpha));
		}
		else
		{
			sprite.setScale({ particle.startScale, particle.startScale });
		}

		sprite.setPosition({ std::floor(particle.position.x), std::floor(particle.position.y) });
		target.draw(sprite);
	}
}