#pragma once

#include "AnimationData.h"

#include <string>
#include <unordered_map>

struct AnimationSet
{
	std::unordered_map<std::string, AnimationData> animations;
};