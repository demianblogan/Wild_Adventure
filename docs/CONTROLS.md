# Controls

The game detects your input device automatically and switches on the fly —
touching a key or a gamepad button switches control hints and hides the
mouse cursor, moving the mouse brings it back. No menu toggle needed.

Move and Jump are fully rebindable in **Settings → Controls → Keyboard**. The
table below is the defaults. Gamepad bindings are fixed.

---

## Keyboard

| Action | Binding |
|---|---|
| Move left | `A` / `←` |
| Move right | `D` / `→` |
| Jump / double jump | `Space` |
| Pause | `Esc` (fixed, not rebindable) |
| Menu navigation | `W` `A` `S` `D` or arrow keys, or mouse |
| Menu confirm | `Enter` / `Space` / Left Mouse Button |
| Menu back | `Esc` / `Backspace` |

While rebinding a key, pressing the gamepad's back button (or `Esc` on
keyboard) cancels out of it instead of getting stuck waiting for a key press.

---

## Xbox controller

| Action | Input |
|---|---|
| Move | D-pad / Left stick |
| Jump / double jump | A |
| Pause | Menu (☰) |
| Menu navigation | D-pad / left stick |
| Menu confirm | A |
| Menu back | B |

---

## PlayStation controller (DualSense / DualShock)

| Action | Input |
|---|---|
| Move | D-pad / Left stick |
| Jump / double jump | Cross (✕) |
| Pause | Options |
| Menu navigation | D-pad / left stick |
| Menu confirm | Cross (✕) |
| Menu back | Circle (◯) |

A DualSense/DualShock has no XInput support at all, so Windows reports its
buttons in Sony's own physical order instead of Xbox's — the game detects the
controller's USB vendor ID and remaps face buttons so Confirm always lands on
Cross and Back on Circle, regardless of which family is plugged in.

---

## DualSense extras

Built on the vendored [`Ohjurot/DualSense-Windows`](https://github.com/Ohjurot/DualSense-Windows)
library, since neither Windows nor XInput expose the extended DualSense
features publicly:

- **Rumble** — two independently-driven motors (a heavier low-frequency one,
  a lighter high-frequency one), blended differently per event so a hit, a
  footstep, a pickup and a menu click each feel distinct rather than just
  louder or quieter. Covers menu navigation/presses/carousels/sliders and
  gameplay: damage, death, landing, footsteps, wall-sliding, fruit pickups,
  box hits, trampolines/arrow boosters, checkpoints, the finish-cup fanfare,
  stomping an enemy, and rock-head impacts.
- **Lightbar** — a warm resting color in menus that briefly flashes brighter
  on every click; in gameplay it tracks your hearts (green at full health,
  orange down a heart, pulsing red on the last one in time with the
  heartbeat vibration) and fades to black as the death animation plays.

Xbox controllers get rumble only — the lightbar calls are no-ops on that
layout. **Vibration** and **Controller Light** can each be turned off
independently in Settings → Gameplay.
