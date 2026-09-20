# Architecture

No game engine. An SFML window and a fixed-timestep loop drive a stack of
states; gameplay runs on a custom sparse-set Entity-Component-System, with a
small retained-mode UI framework built on top of the same rendering layer for
every menu. Roughly 20,000 lines of C++23 across ~240 files.

For how the common game-programming patterns map onto this code, see
[PATTERNS.md](PATTERNS.md).

---

## The loop

`Application::Run` accumulates real time and drains it in fixed 1/60s steps,
rather than feeding a variable delta straight into `Update`:

```cpp
sf::Clock clock;
float remainderTime = 0.0f;

while (window.isOpen())
{
    float frameTime = clock.restart().asSeconds();
    if (frameTime > MaxFrameTime)
        frameTime = MaxFrameTime;             // clamp a stall instead of a catch-up burst

    ProcessEvents();

    if (isWindowFocused)
    {
        remainderTime += frameTime;
        while (remainderTime >= FixedDeltaTime)
        {
            Update(FixedDeltaTime);           // always exactly 1/60s
            remainderTime -= FixedDeltaTime;
        }
    }
    else
    {
        remainderTime = 0.0f;                 // unfocused: freeze instead of catching up
        sf::sleep(sf::seconds(UnfocusedSleepInterval));
    }

    Render(remainderTime / FixedDeltaTime);   // interpolation factor between the last two steps
}
```

Every `Update` call sees the exact same `deltaTime`, so physics and collision
never have to account for a variable step. `Render` still runs once per real
frame (so the game feels as smooth as the display's refresh rate) by
interpolating each renderable's position between its previous and current
fixed-step transform — see `ECS::PreviousTransform` and `RenderSystem`.
`MaxFrameTime` (0.25s) is the safety valve: if the game stalls (alt-tab, a
Windows dialog), the accumulator is capped so it drains as one slow catch-up
step instead of resimulating an arbitrarily long backlog of physics.

`Render` draws into an off-screen virtual canvas (`VirtualScreen`, a fixed
480×270) which is then scaled up to the real window — every screen is
authored and laid out at that one resolution regardless of the window's
actual size.

---

## Layers

| Layer | Folder | Responsibility |
|---|---|---|
| Bootstrap | `src/` root (`main.cpp`, `Application.*`, `Context.h`) | Entry point, the fixed-timestep loop, and `Context` — one struct of references to every shared service, handed to every state/screen instead of threading a dozen constructor parameters everywhere |
| ECS core | `core/ecs/` | The sparse-set `Registry` and `ComponentPool<T>` |
| Core services | `core/` | Input (actions, rebinding, gamepad remap), Settings, Campaign (save data), GamepadHaptics, Resources, Camera, VirtualScreen |
| Components | `components/` | Plain-data structs only, grouped by domain: `physics`, `render`, `combat`, `items`, `traps`, `ai`, `tags` |
| Systems | `systems/` | All per-frame gameplay logic — physics, enemy AI (one file per enemy type), traps, damage, pickups — each operating on components via `Registry::ForEach` |
| Level | `level/` | `LevelSequencer` (the reveal → play → finish → complete phase machine and checkpoints), `LevelSetup` (theme/JSON parsing), `PlayerFeedbackController` (the player's cosmetic reactions: squash & stretch, dust, camera shake, haptics) |
| Tilemap | `tilemap/` | Tiled `.tmj` loading, the collision-query `Tilemap`, and `TilemapRenderer` |
| States | `states/` | Every screen on the `StateMachine` stack — `GameState`, `MenuState`, `PauseState`, `LevelCompleteState`, `CampaignVictoryState`, the splash/language-picker states |
| Screens | `screens/` | Larger composed pieces states are built from: `HUD`, `SettingsController`, `CharacterSelectController`, `SelectLevelController`, `MenuBackdrop`, `TitleDropAnimation` |
| UI | `ui/` | The retained-mode widget framework (`Element`, `Button`, `Checkbox`, `Slider`, `Stepper`, `Label`, `Root`) and `DataLoader`, which builds a tree of them from JSON |
| Audio | `audio/` | `Mixer`, `MusicPlayer`, `SoundPlayer` |
| Localization | `localization/` | `Language`, `LocalizationManager` — one string catalog per language, English as the fallback for any missing key |
| Graphics | `graphics/` | Cosmetic/rendering helpers shared across states: `ParticleSystem`, `ConfettiSystem`, `LightOverlay`, `ScreenShake`, `ColorGrading`, `Transition`, `AnimatedBackground` |

---

## The ECS

A classic sparse-set implementation (`core/ecs/`), not a third-party library.
`ComponentPool<T>` holds a packed array of `T` plus an entity → index map, so
adding/removing is O(1) (swap-with-last) and iterating a component type never
walks a gap. `Registry` owns one pool per type, created lazily on first use,
and exposes:

```cpp
Entity entity = registry.CreateEntity();
registry.Add<Health>(entity, { .current = 3, .maximum = 3 });
registry.Has<Health>(entity);
registry.Get<Health>(entity).current -= 1;
registry.RemoveFrom<Health>(entity);
registry.DestroyEntity(entity); // removes it from every pool it was in

registry.ForEach<Player, Transform, Health>(
    [](Entity e, Player&, Transform& t, Health& h) { /* ... */ });
```

`ForEach<First, Rest...>` iterates `First`'s packed array and filters each
entity against `Has<Rest>...` before invoking the callback — no separate
"view" object to build first. Entities are plain integer IDs, never reused
within one `Registry`'s lifetime; a level attempt gets a fresh `Registry`
(and `SceneLoader`) rather than trying to reset one in place.

---

## Gameplay systems

Every enemy type is its own system (`BeeSystem`, `ChickenSystem`, `GhostSystem`,
`PlantSystem`, `ShellSystem`, `SnailSystem`, `TrunkSystem`, `TurtleSystem`) plus
two shared movement systems (`PatrolSystem`, `GroundPatrolSystem`) that a
couple of the simpler enemies drive their movement through instead of
duplicating it. Stomping is unified: `EnemyDeathSystem` runs the shared
freeze → fall → despawn sequence for anything tagged `EnemyDeath`; an enemy
that needs a different reaction to being stomped (the snail turning into a
shell) opts out with a `StompCustomDeath`/`Stomped` tag pair instead, and an
enemy that shouldn't die from a stomp at all (the turtle while spiked) is
tagged `Spiky`, which the shared systems check for before applying the
default reaction.

`PlayerFeedbackController` centralizes the player's own cosmetic reactions —
squash & stretch, run dust, camera shake, and every haptic/lightbar cue tied
to the player's own state — by comparing this frame's health/jump-count/wall-
contact against what it saw last frame. That diff-against-last-frame is the
one piece of state this class exists to own, so no other system has to also
remember "was the player already on the ground a moment ago."

---

## Data-driven content

Levels are Tiled `.tmj` maps (`TilemapLoader`); entities, UI screens and
prefabs are JSON (`SceneLoader`, `ui/DataLoader`) rather than code, so
levels/menus can be edited without a rebuild. Gamepad haptics are the
exception: `core/HapticCues.h` keeps every vibration/lightbar tuning value as
named `constexpr`s in code rather than an external file — deliberately, since
unlike level layout or UI text, retuning "how strong does a footstep buzz
feel" is a one-line code edit that never needs a design-time hot-reload loop.

---

## Persistence

Three files live under `%LOCALAPPDATA%\Alone Bull Company\Wild Adventure\`
(`AppDataPath::Resolve`, falling back to a `user_data\` folder beside the
executable if `LOCALAPPDATA` can't be read): `settings.json`, `save.json`,
`input.json`. All three go through `SafeFileWrite` — write to a temp file,
then atomically replace the real one, so a crash or a full disk mid-write
can't leave a half-written file behind. A file that exists but fails to
parse is renamed to `<name>.corrupt` rather than deleted or silently
overwritten, and the game falls back to defaults for that file instead of
refusing to start.

---

## Localization

`LocalizationManager` loads one plain-text catalog per `Language` (`section.key = value`
lines) with English loaded first as the fallback, so a partially translated
language still shows English for any key it's missing rather than a blank
string or a raw key name. A revision counter bumps on every language change;
UI built from cached `sf::Text` (which doesn't re-resolve its string every
frame) reloads from JSON when it notices its own last-seen revision is stale,
picking up the new language immediately instead of on the next screen visit.
