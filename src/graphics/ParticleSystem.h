#pragma once

#include <SFML/System/Vector2.hpp>

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

	// Run dust kicked up behind a mover, spawned at the back of its feet (the side
	// opposite travel). direction is the horizontal sign: +1 right, -1 left.
	void EmitRunDust(sf::Vector2f feet, int direction);

	// One animated wisp trailing off the ghost's back (flies out the opposite way from
	// facingDir and fades, cycling through its frames over its short life).
	void EmitGhostTrail(sf::Vector2f position, int facingDir);

	// One rising water bubble (drawn procedurally, no texture) that drifts upward,
	// wobbling sideways, and fades out — ambient detail for a water level.
	void EmitBubble(sf::Vector2f position);

private:
	enum class Kind { Dust, Debris, Bubble };
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
		bool animated = false; // play through the frames over the lifetime instead of holding frameIndex
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

	Resources& resources;
	const Tilemap* tilemap = nullptr;
	std::vector<Particle> particles;

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