#pragma once

// Lets the menu ask the application to apply graphics settings to the real window,
// without exposing the whole Application.
class GraphicsTarget
{
public:
	virtual ~GraphicsTarget() = default;

	virtual void ApplyGraphics() = 0; // recreate window from current settings
	virtual void ApplyVsync() = 0;    // vsync only (cheap, no window recreate)
};