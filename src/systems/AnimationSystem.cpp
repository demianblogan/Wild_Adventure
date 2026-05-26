#include "AnimationSystem.h"

#include "components/Animation.h"
#include "core/ecs/Registry.h"

AnimationSystem::AnimationSystem(Registry& registry)
	: registry(registry)
{}

void AnimationSystem::Update(float deltaTime)
{
	registry.ForEach<Animation>(
		[deltaTime](Entity entity, Animation& animation)
		{
			animation.elapsedTime += deltaTime;

			if (animation.elapsedTime < animation.frameDuration)
				return;

			animation.elapsedTime -= animation.frameDuration;
			animation.currentFrame++;

			if (animation.currentFrame >= animation.frameCount)
			{
				if (animation.isLooping)
					animation.currentFrame = 0;
				else
					animation.currentFrame = animation.frameCount - 1;
			}
		});
}