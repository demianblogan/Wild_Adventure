#include "ConfettiSystem.h"

#include "core/Random.h"
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
	const std::string Texture = "confetti";
	constexpr float TwoPi = 6.28318530718f;
}

ConfettiSystem::ConfettiSystem(Resources& resources)
	: resources(resources)
{}

void ConfettiSystem::Emit(sf::Vector2f center)
{
	for (int i = 0; i < PiecesPerBurst; i++)
	{
		Confetto piece;
		piece.baseX = center.x + Random::Float(-SpawnWidth, SpawnWidth);
		piece.y = center.y - SpawnRise + Random::Float(-SpawnBand, SpawnBand);

		piece.fallSpeed = Random::Float(18.0f, 38.0f);

		piece.swayAmplitude = Random::Float(5.0f, 14.0f);
		piece.swayFrequency = Random::Float(1.5f, 3.0f);
		piece.swayPhase = Random::Float(0.0f, TwoPi);

		piece.angle = Random::Float(0.0f, 360.0f);
		piece.angularSpeed = Random::Float(80.0f, 200.0f) * (Random::Int(0, 1) == 0 ? -1.0f : 1.0f);

		piece.flipFrequency = Random::Float(2.0f, 4.0f);
		piece.flipPhase = Random::Float(0.0f, TwoPi);

		piece.frame = Random::Int(0, FrameCount - 1);
		piece.lifetime = Lifetime;

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

	const sf::Texture& texture = resources.textures.Get(Texture);

	sf::Sprite sprite(texture);
	sprite.setOrigin({ FrameSize / 2.0f, FrameSize / 2.0f });

	for (const Confetto& piece : confetti)
	{
		sprite.setTextureRect(sf::IntRect({ piece.frame * FrameSize, 0 }, { FrameSize, FrameSize }));

		const float x = piece.baseX + piece.swayAmplitude * std::sin(piece.swayFrequency * piece.time + piece.swayPhase);
		sprite.setPosition({ std::floor(x), std::floor(piece.y) });
		sprite.setRotation(sf::degrees(piece.angle));

		// scale.x sweeps -1..1, so the piece appears to turn over (edge-on at 0).
		const float scaleX = std::sin(piece.flipFrequency * piece.time + piece.flipPhase);
		sprite.setScale({ scaleX * Scale, Scale });

		float alpha = 1.0f;
		const float remaining = piece.lifetime - piece.age;
		if (remaining < FadeTime)
			alpha = std::max(0.0f, remaining / FadeTime);

		sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 255.0f)));

		target.draw(sprite);
	}
}