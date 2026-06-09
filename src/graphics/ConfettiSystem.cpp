#include "ConfettiSystem.h"

#include "core/Resources.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Angle.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

namespace
{
	const std::string TEXTURE = "confetti";
	constexpr float TWO_PI = 6.28318530718f;
}

ConfettiSystem::ConfettiSystem(Resources& resources)
	: resources(resources)
{}

float ConfettiSystem::RandomFloat(float min, float max)
{
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine);
}

int ConfettiSystem::RandomInt(int min, int max)
{
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(randomEngine);
}

void ConfettiSystem::Emit(sf::Vector2f center)
{
	for (int i = 0; i < PIECES_PER_BURST; i++)
	{
		Confetto piece;
		piece.baseX = center.x + RandomFloat(-SPAWN_WIDTH, SPAWN_WIDTH);
		piece.y = center.y - SPAWN_RISE + RandomFloat(-SPAWN_BAND, SPAWN_BAND);

		piece.fallSpeed = RandomFloat(18.0f, 38.0f);

		piece.swayAmplitude = RandomFloat(5.0f, 14.0f);
		piece.swayFrequency = RandomFloat(1.5f, 3.0f);
		piece.swayPhase = RandomFloat(0.0f, TWO_PI);

		piece.angle = RandomFloat(0.0f, 360.0f);
		piece.angularSpeed = RandomFloat(80.0f, 200.0f) * (RandomInt(0, 1) == 0 ? -1.0f : 1.0f);

		piece.flipFrequency = RandomFloat(2.0f, 4.0f);
		piece.flipPhase = RandomFloat(0.0f, TWO_PI);

		piece.frame = RandomInt(0, FRAME_COUNT - 1);
		piece.lifetime = LIFETIME;

		confetti.push_back(piece);
	}
}

void ConfettiSystem::Update(float deltaTime)
{
	for (Confetto& piece : confetti)
	{
		piece.time += deltaTime;
		piece.age += deltaTime;
		piece.y += piece.fallSpeed * deltaTime;
		piece.angle += piece.angularSpeed * deltaTime;
	}

	confetti.erase(
		std::remove_if(confetti.begin(), confetti.end(),
			[](const Confetto& piece) { return piece.age >= piece.lifetime; }),
		confetti.end());
}

void ConfettiSystem::Draw(sf::RenderTarget& target)
{
	if (confetti.empty())
		return;

	const sf::Texture& texture = resources.textures.Get(TEXTURE);

	sf::Sprite sprite(texture);
	sprite.setOrigin({ FRAME_SIZE / 2.0f, FRAME_SIZE / 2.0f });

	for (const Confetto& piece : confetti)
	{
		sprite.setTextureRect(sf::IntRect({ piece.frame * FRAME_SIZE, 0 }, { FRAME_SIZE, FRAME_SIZE }));

		const float x = piece.baseX + piece.swayAmplitude * std::sin(piece.swayFrequency * piece.time + piece.swayPhase);
		sprite.setPosition({ std::floor(x), std::floor(piece.y) });
		sprite.setRotation(sf::degrees(piece.angle));

		// scale.x sweeps -1..1, so the piece appears to turn over (edge-on at 0).
		const float scaleX = std::sin(piece.flipFrequency * piece.time + piece.flipPhase);
		sprite.setScale({ scaleX * SCALE, SCALE });

		float alpha = 1.0f;
		const float remaining = piece.lifetime - piece.age;
		if (remaining < FADE_TIME)
			alpha = std::max(0.0f, remaining / FADE_TIME);

		sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 255.0f)));

		target.draw(sprite);
	}
}