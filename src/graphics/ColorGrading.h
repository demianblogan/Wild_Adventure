#pragma once

#include <SFML/Graphics/Glsl.hpp>

#include <string>

// Parameters for the color grading post-effect. Defaults are identity:
// the shader pass is skipped entirely when nothing deviates from them.
struct ColorGrading
{
	sf::Glsl::Vec3 tint = { 1.0f, 1.0f, 1.0f }; // per-channel multiplier
	float brightness = 1.0f;                    // linear multiplier
	float saturation = 1.0f;                    // 0 = grayscale, 1 = unchanged
	float contrast = 1.0f;                      // pivot at mid-gray

	bool IsIdentity() const;
};

// Reads data/levels/color_grading.json: entries keyed by level theme ("green",
// "hot", "sweet", ...), with a "default" fallback. The theme comes from the
// level's map property. Missing file or entry yields identity grading.
ColorGrading LoadColorGrading(const std::string& theme);
