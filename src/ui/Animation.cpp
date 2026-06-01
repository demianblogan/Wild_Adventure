#include "Animation.h"

#include <cmath>
#include <numbers>

namespace UI
{
	Animation::Animation(float fromValue, float toValue, float duration,
		AnimationCurve curve, AnimationLoop loop,
		std::function<void(float)> setter)
		: fromValue(fromValue)
		, toValue(toValue)
		, duration(duration)
		, curve(curve)
		, loop(loop)
		, setter(std::move(setter))
	{}

	void Animation::Update(float deltaTime)
	{
		if (isFinished)
			return;

		elapsed += deltaTime;

		// Time elapsed divided by duration. 0.0 at start, 1.0 at end of one pass,
		// can grow beyond 1.0 for Loop and PingPong over multiple passes.
		const float elapsedRatio = (duration > 0.0f) ? (elapsed / duration) : 1.0f;

		// Position within a single from -> to pass, always in [0.0, 1.0].
		// Loop/PingPong logic above wraps elapsedRatio into this range.
		float passPosition = 0.0f;

		switch (loop)
		{
		case AnimationLoop::Once:
			// Single pass: passPosition follows elapsedRatio until it reaches 1.0, then animation ends.
			// elapsedRatio = 0.3 -> passPosition = 0.3; elapsedRatio >= 1.0 -> passPosition = 1.0, isFinished.
			if (elapsedRatio >= 1.0f)
			{
				passPosition = 1.0f;
				isFinished = true;
			}
			else
			{
				passPosition = elapsedRatio;
			}
			break;

		case AnimationLoop::Loop:
			// Take fractional part of elapsedRatio: drops the integer count of completed passes.
			// elapsedRatio = 1.7 -> passPosition = 0.7 (second pass, 70% through it).
			// Causes a visual jump from 1.0 back to 0.0 at each cycle boundary.
			passPosition = elapsedRatio - std::floor(elapsedRatio);
			break;

		case AnimationLoop::PingPong:
		{
			// Wrap elapsedRatio into [0, 2): the first half [0, 1) is forward, the second half [1, 2) is backward.
			// elapsedRatio = 2.3 -> cycle = 0.3 (forward phase, going from -> to).
			// elapsedRatio = 1.7 -> cycle = 1.7 (backward phase, going to -> from).
			const float cycle = elapsedRatio - std::floor(elapsedRatio / 2.0f) * 2.0f;

			// If we're in the forward half, passPosition = cycle directly (0 -> 1).
			// If we're in the backward half, mirror it: 2.0 - cycle gives 1 -> 0 as cycle goes 1 -> 2.
			passPosition = (cycle <= 1.0f) ? cycle : (2.0f - cycle);
			break;
		}
		}

		// Apply the easing curve to passPosition: e.g. 0.5 may become 0.5 (Linear), 0.5 (Sine), or 0.75 (EaseOut).
		// This warps the "where we are in the pass" value to make the motion feel different.
		const float curved = ApplyCurve(passPosition);

		// Linear interpolation between fromValue and toValue based on curved position.
		// curved = 0 -> value = fromValue; curved = 1 -> value = toValue; curved = 0.5 -> midpoint between them.
		const float value = fromValue + (toValue - fromValue) * curved;

		if (setter)
			setter(value);

		if (isFinished && onFinished)
		{
			onFinished();
			onFinished = nullptr;
		}
	}

	void Animation::SetOnFinished(std::function<void()> callback)
	{
		onFinished = std::move(callback);
	}

	float Animation::ApplyCurve(float time) const
	{
		switch (curve)
		{
		case AnimationCurve::Linear:
			// Returns time as-is: input 0 -> 0, 0.5 -> 0.5, 1 -> 1. Constant speed.
			return time;

		case AnimationCurve::Sine:
			// Maps [0, 1] onto a half-cosine wave: 0 -> 0, 0.5 -> 0.5, 1 -> 1,
			// but with smooth acceleration near 0 and smooth deceleration near 1.
			return 0.5f - 0.5f * std::cos(time * std::numbers::pi_v<float>);

		case AnimationCurve::EaseOut:
			// Inverted quadratic: 0 -> 0, 0.5 -> 0.75, 1 -> 1.
			// Fast at the start, slows down towards the end.
			return 1.0f - (1.0f - time) * (1.0f - time);

		default:
			return time;
		}
	}
}