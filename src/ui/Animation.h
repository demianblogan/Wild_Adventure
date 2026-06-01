#pragma once

#include <functional>

namespace UI
{
	enum class AnimationCurve
	{
		Linear,    // uniform change (constant speed)
		Sine,      // smooth start and smooth end (ease-in-out)
		EaseOut    // fast start, gradual slowdown towards the end
	};

	enum class AnimationLoop
	{
		Once,      // play from -> to once, then finish
		Loop,      // repeat from -> to, jumps back to from on each cycle
		PingPong   // smooth from -> to -> from -> to -> ...
	};

	class Animation
	{
	public:
		Animation(float fromValue, float toValue, float duration,
			AnimationCurve curve, AnimationLoop loop,
			std::function<void(float)> setter);

		void Update(float deltaTime);

		bool IsFinished() const { return isFinished; }

		void SetOnFinished(std::function<void()> callback);

	private:
		float ApplyCurve(float t) const;

		float fromValue;
		float toValue;
		float duration;
		AnimationCurve curve;
		AnimationLoop loop;
		std::function<void(float)> setter;
		std::function<void()> onFinished;

		float elapsed = 0.0f;
		bool isFinished = false;
	};
}