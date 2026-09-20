# Design patterns

How this codebase maps onto the common game-programming patterns — what each
one solves, how it's built here, and what it costs. Reference frame: Robert
Nystrom's *Game Programming Patterns*.

- [Program-shaping patterns](#program-shaping-patterns)
- [Screens and flow](#screens-and-flow)
- [Data and resources](#data-and-resources)
- [Simulation and reaction](#simulation-and-reaction)
- [Input](#input)
- [Deliberately not used](#deliberately-not-used)
- [Summary](#summary)

---

## Program-shaping patterns

### Game Loop (fixed timestep)

**Problem.** A game can't block waiting for input like a console program —
and physics/collision code gets noticeably harder to reason about (and easier
to get wrong) if the same move can be simulated over a slightly different
`deltaTime` from one frame to the next.

**Here.** [`Application::Run`](../src/Application.cpp) accumulates real
elapsed time and drains it in fixed `1/60`s steps, capping the accumulator so
a stall (alt-tab, a Windows dialog) produces one slow catch-up step instead
of resimulating an arbitrarily long backlog:

```cpp
remainderTime += frameTime;
while (remainderTime >= FixedDeltaTime)
{
    Update(FixedDeltaTime);
    remainderTime -= FixedDeltaTime;
}
Render(remainderTime / FixedDeltaTime); // interpolate for a smooth render
```

Every gameplay system always sees exactly the same `deltaTime`, and
`Render` still runs once per real frame by interpolating between the last
two fixed steps (`ECS::PreviousTransform` + `RenderSystem`), so simulation
stability and render smoothness don't have to trade off against each other.

### Service Locator

**Problem.** Nearly every state and screen needs the window, resources,
audio, settings, the ECS registry, the state machine itself — passing each
one through every constructor individually turns every signature into a wall
of parameters, and adding one new shared service means touching every call
site that builds a state.

**Here.** [`Context`](../src/Context.h) is one struct of references, built
once in `Application` and handed to every state/screen by reference:

```cpp
struct Context
{
    VirtualScreen& virtualScreen;
    StateMachine& stateMachine;
    Resources& resources;
    Audio::Mixer& audioMixer;
    Input& input;
    Settings& settings;
    // ... every other shared service, by reference
};
```

A state stores its `Context&` and reaches into it for whatever it needs —
`context.audioMixer.PlaySound(...)`, `context.gamepadHaptics.PulseVibration(...)`.
Nothing is a global; everything is still owned by `Application` and threaded
through explicitly, so ownership and lifetime stay obvious, but a state never
has to declare "I also need the campaign save data" in its constructor
signature just because one deeply nested call needs it.

**Cost.** A `Context&` member makes any class holding one look like it could
touch *anything* — a real discoverability cost versus tightly scoped
dependencies. In practice most classes only ever touch a handful of its
fields.

---

## Screens and flow

### State (as a stack)

**Problem.** Pause sits *on top of* gameplay, not instead of it — resuming
doesn't rebuild the level — and the settings screen is reachable both from
the main menu and from pause, each time returning to wherever it was opened
from.

**Here.** [`StateMachine`](../src/core/StateMachine.h) owns a stack of
`State`s. `GameState`, `MenuState`, `PauseState`, `LevelCompleteState`,
`CampaignVictoryState` and the splash/language states each override
`HandleEvent`/`Update`/`Render`; a state declares whether the one below it
should still render (`IsRenderingStateBelow`) and/or still update
(`IsUpdatingStateBelow`) while it's on top — `PauseState` renders the frozen
game behind it but doesn't update it, for instance.

Pushes/pops queue as pending actions and are only applied once, from
`StateMachine::Update`, after every state on the stack has already had its
turn that frame — so a state popping *itself* from inside its own event
handler (Pause's "Continue" button) never risks running more code on an
object that's already been destroyed mid-call.

### Pending navigation request

**Problem.** `SettingsController` (shared by the main menu and pause) reacts
to a button press by reloading a different JSON panel or leaving the
settings screen entirely — but that has to happen *between* frames, not from
inside the JSON-driven UI's own event callback, or it would be mutating the
very `UI::Root` that's still iterating its widgets to dispatch the event.

**Here.** Button/action callbacks only ever set a `pendingRequest` enum
(`NavRequest::OpenPanel`, `::Back`, `::Save`, ...) plus whatever data it
needs (`pendingPanelId`); `ApplyPendingNavigation()`, called once per `Update`
after the UI has finished handling this frame's input, is the only place
that actually swaps panels or reports "closed" back to the owning state. The
same shape shows up in `MenuState` and `LevelCompleteState` for their own
button-driven transitions.

---

## Data and resources

### Entity-Component-System

**Problem.** Nine enemy types, four trap types, pickups, checkpoints and the
player all need independent combinations of position, collision, health,
animation and AI state — a class hierarchy trying to cover that combinatorial
spread turns into either a deep inheritance tree or a pile of unrelated
one-off classes, and neither shares the physics/rendering code the others
need.

**Here.** A hand-written sparse-set ECS ([`core/ecs/`](../src/core/ecs)):
`Registry::Add<T>`/`Has<T>`/`Get<T>`/`RemoveFrom<T>` on plain-data
`components/` structs, and `Registry::ForEach<Types...>` for every system to
iterate exactly the entities that have the components it cares about — a
`BeeSystem` only ever sees entities with `Bee` + `Transform` + `Velocity` +
..., regardless of what else exists in the level. See
[ARCHITECTURE.md#the-ecs](ARCHITECTURE.md#the-ecs) for the pool internals.

### Type Object / data-driven content

**Problem.** Level layouts, enemy/trap prefabs and every menu screen's layout
are the kind of content that changes constantly during development and
shouldn't need a recompile to iterate on.

**Here.** Levels are Tiled `.tmj` maps (`TilemapLoader`); entities are JSON
prefabs assembled by `SceneLoader` into whatever component set they declare;
every menu is a JSON tree of `Element`/`Button`/`Checkbox`/... built by
[`UI::DataLoader`](../src/ui/DataLoader.cpp), which resolves named actions
registered in code (`RegisterAction("menu_save", ...)`) rather than hardcoding
what a given button does. Gamepad haptic tuning is the deliberate exception —
see [ARCHITECTURE.md#data-driven-content](ARCHITECTURE.md#data-driven-content)
for why that one stays in code.

### Facade (resource access)

**Problem.** Loading, caching and looking up textures/fonts/sounds by name
shouldn't leak `sf::Texture` construction details or file paths into every
system and screen that just wants "the bee spritesheet."

**Here.** [`ResourceManager<Resource>`](../src/core/ResourceManager.h) is one
small generic cache (`Resources::textures`, `::fonts`, `::music`, ... are
instantiations of it); a caller just does
`resources.textures.Get("bee_idle")`. Load order and file paths live in each
screen's own setup code or a level's JSON, not scattered as raw path strings
through gameplay logic.

---

## Simulation and reaction

### Update Method

**Problem.** Every active object — an enemy, a trap, a particle burst, the
camera shake — needs to advance itself once a frame, and that logic has to
live *with* the object's own system, not in one giant switch inside
`GameState`.

**Here.** `Update(deltaTime)` (or, for ECS systems, `Update()`/`Update(deltaTime)`
over a `Registry::ForEach`) is the interface every system and every `UI::`
widget implements. `GameState::Update` is mostly calling `Update` on its
~30 systems in a fixed order, then reacting to what changed.

### Frame-to-frame diffing (a lightweight alternative to Observer)

**Problem.** Landing, jumping, taking damage and losing a heart all need a
cosmetic reaction (a squash, a camera shake, a haptic pulse) — but wiring a
callback/event for each from deep inside physics and damage code would mean
those systems reaching out to sound/haptics/particles directly, coupling
gameplay rules to presentation.

**Here.** [`PlayerFeedbackController::Update`](../src/level/PlayerFeedbackController.cpp)
is handed the player's current `Health`/`Jump`/`CollisionState` every
frame and compares each against what it saw last frame:

```cpp
if (!wasOnGround && onGround)          // landed just now
    particles.Emit("land", feet);

if (health.current < previousPlayerHealth) // took damage just now
    camera.Shake(ShakeHit);
```

No system that changes health or ground state has to know feedback exists;
this one class owns the "what changed" question and every reaction to it,
which is also exactly what makes it easy to gate each reaction behind its own
Settings → Gameplay toggle (hit-stop, camera shake, the vignette) in one
place instead of at each place damage/landing/jumping could originate.

---

## Input

### Command-ish input, not a bare key check

**Problem.** `if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))` scattered
through gameplay code can't be rebound, can't distinguish keyboard from
gamepad, and can't apply the DualSense's different physical button layout
without every call site knowing about it.

**Here.** [`Action`](../src/core/Input.h) is a project-defined enum
(`MoveLeft`, `Jump`, `MenuConfirm`, ...); `Input::WasPressed(Action)` /
`IsDown(Action)` are what gameplay and menu code actually call.
`Input::Update()` resolves every action's current binding list (keys,
gamepad buttons, gamepad axes) once a frame, including the vendor-specific
DualSense button remap, so nothing above that layer ever touches `sf::Keyboard`
or `sf::Joystick` directly — which is what makes keyboard rebinding, and
"Confirm always lands on Cross regardless of controller," both live entirely
inside `Input` instead of leaking into every screen that reads input.

---

## Deliberately not used

**Object Pool for particles.** Dust, confetti and debris (`ParticleSystem`,
`ConfettiSystem`) are plain `std::vector`s that grow and shrink normally. At
this scale (short-lived bursts of a handful to a few dozen particles) the
allocator churn has never shown up as a cost worth a pool's complexity;
revisit if a profiler ever says otherwise.

**Singleton.** Every shared service (`Settings`, `Campaign`, `Audio::Mixer`,
`GamepadHaptics`, ...) is a normal object owned by `Application` and reached
through `Context`, not a static/global instance — one extra indirection
(`context.settings` instead of `Settings::Instance()`) in exchange for
explicit ownership and no static-init-order surprises.

---

## Summary

| Pattern | Where | Why |
|---|---|---|
| Game Loop (fixed timestep) | `Application::Run` | Deterministic simulation step, smooth interpolated render, stall-safe |
| Service Locator | `Context` | One reference bundle instead of N constructor params |
| State (stack) | `StateMachine` | Pause-over-gameplay, settings reachable from two different parents |
| Pending navigation request | `SettingsController`, `MenuState` | A button callback can safely change screens without mutating the UI tree mid-dispatch |
| Entity-Component-System | `core/ecs/`, `systems/` | 9 enemy types + traps + pickups share physics/rendering without a combinatorial class hierarchy |
| Type Object / data-driven | `SceneLoader`, `UI::DataLoader`, Tiled maps | Levels, prefabs and menus editable without a rebuild |
| Facade | `ResourceManager<Resource>` | Asset lookup by name, load details hidden from callers |
| Update Method | every `Update(deltaTime)` | Per-object logic stays with the object |
| Frame-to-frame diffing | `PlayerFeedbackController` | Cosmetic reactions decoupled from the gameplay code that causes them |
| Command-ish input | `Action` / `Input` | Rebindable, device-agnostic actions instead of hardcoded key checks |
