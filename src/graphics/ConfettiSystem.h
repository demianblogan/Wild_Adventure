#pragma once

#include <SFML/System/Vector2.hpp>

#include <random>
#include <vector>

struct Resources;

namespace sf
{
	class RenderTarget;
}

// A purely cosmetic burst of confetti that flutters down like falling leaves and
// fades out. It is independent of the ECS and physics: confetti collide with
// nothing and are drawn on top of the world.
class ConfettiSystem
{
public:
	ConfettiSystem(Resources& resources);

	void Emit(sf::Vector2f center);
	void Update(float deltaTime);
	void Draw(sf::RenderTarget& target);

private:
	struct Confetto
	{
		float baseX = 0.0f;
		float y = 0.0f;
		float fallSpeed = 0.0f;

		float swayAmplitude = 0.0f; // horizontal drift, like a leaf
		float swayFrequency = 0.0f;
		float swayPhase = 0.0f;

		float angle = 0.0f;
		float angularSpeed = 0.0f;  // constant spin, random direction

		float flipFrequency = 0.0f; // scale.x oscillates -1..1 for the "turning over" look
		float flipPhase = 0.0f;

		int frame = 0;              // which colour (spritesheet cell)
		float time = 0.0f;
		float age = 0.0f;
		float lifetime = 0.0f;
	};

	float RandomFloat(float min, float max);
	int RandomInt(int min, int max);

	Resources& resources;
	std::vector<Confetto> confetti;
	std::mt19937 randomEngine{ std::random_device{}() };

	static constexpr int FRAME_SIZE = 16;
	static constexpr float SCALE = 0.3f;       // visual size of each piece (1.0 = native 16px)

	static constexpr int FRAME_COUNT = 6;
	static constexpr int PIECES_PER_BURST = 16;
	
	static constexpr float SPAWN_WIDTH = 30.0f; // horizontal half-extent of the spawn band
	static constexpr float SPAWN_RISE = 40.0f;  // how far above the point they start
	static constexpr float SPAWN_BAND = 12.0f;  // vertical spread of the starting band

	static constexpr float LIFETIME = 5.0f;
	static constexpr float FADE_TIME = 1.5f;  // fade-out window at the end of life
};