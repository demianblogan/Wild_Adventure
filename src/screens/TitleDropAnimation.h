#pragma once

#include <functional>
#include <string>

namespace UI
{
	class Element;
}

struct Resources;
class ScreenShake;

namespace Haptics
{
	class GamepadHaptics;
}

// Splits an ASCII title (the game's brand name is never localized) into
// individual letters and launches them into `container`, one after another,
// left to right: each starts huge and tilted at the title block's own
// center -- as if flung out from right in front of the camera -- then
// shrinks, untilts and slides into its actual place in the word. Landing
// kicks the screen (via `shake`) and scatters a burst of pixel dust, plus a
// controller vibration pulse that grows stronger letter by letter (see
// core/HapticCues.h). `container` must already be positioned/sized by its
// parent (anchor/pivot/offset) -- this only fills in its size and children.
// onAllLanded fires once the last letter has finished settling.
void BuildTitleDropAnimation(UI::Element& container, Resources& resources, ScreenShake& shake,
	Haptics::GamepadHaptics& haptics, const std::string& asciiText, std::function<void()> onAllLanded);
