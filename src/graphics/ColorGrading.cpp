#include "ColorGrading.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>

bool ColorGrading::IsIdentity() const
{
	return tint == sf::Glsl::Vec3{ 1.0f, 1.0f, 1.0f }
		&& brightness == 1.0f
		&& saturation == 1.0f
		&& contrast == 1.0f;
}

ColorGrading LoadColorGrading(const std::string& theme)
{
	ColorGrading grading;

	std::ifstream file("data/levels/color_grading.json");
	if (!file.is_open())
		return grading;

	const nlohmann::json data = nlohmann::json::parse(file);

	const nlohmann::json* entry = nullptr;
	if (data.contains(theme))
		entry = &data.at(theme);
	else if (data.contains("default"))
		entry = &data.at("default");

	if (entry == nullptr)
		return grading;

	if (entry->contains("tint"))
	{
		const auto& tint = entry->at("tint");
		grading.tint = { tint.at(0).get<float>(), tint.at(1).get<float>(), tint.at(2).get<float>() };
	}

	grading.brightness = entry->value("brightness", 1.0f);
	grading.saturation = entry->value("saturation", 1.0f);
	grading.contrast = entry->value("contrast", 1.0f);

	return grading;
}
