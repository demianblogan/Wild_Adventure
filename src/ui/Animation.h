#pragma once

#include <functional>

namespace UI
{
	enum class AnimationCurve
	{
		Linear,    // uniform change (constant speed)
		Sine,      // smooth start and smooth end (ease-in-out)
		EaseOut,   // fast start, gradual slowdown towards the end
		EaseIn     // slow start, accelerating hard towards the end (gravity/falling feel)
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
		// delay: seconds to wait before the animation starts; the setter is
		// not called at all during the wait, so the property stays at
		// whatever it was set to before this animation was added.
		Animation(float fromValue, float toValue, float duration,
			AnimationCurve curve, AnimationLoop loop,
			std::function<void(float)> setter, float delay = 0.0f);

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

		float delay;
		float elapsed = 0.0f;
		bool isFinished = false;
	};
}