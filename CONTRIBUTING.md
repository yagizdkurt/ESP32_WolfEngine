# Contributing to WolfEngine

WolfEngine is a game engine for the ESP32. It's still evolving, so contributions of all kinds are genuinely useful — whether that's fixing a bug you ran into, adding a subsystem you needed, or just improving a confusing comment.

---

## How Can I Contribute?

**Found a bug?** Open an issue. Include `WE_Settings.hpp` flags you have enabled, and a short description of what you expected vs what happened. The more specific, the better.

**Want to fix something?** Go for it. For small fixes — a wrong accessor, a missing tick flag check, a comment that's gone stale — just open a PR. For anything larger, a quick issue comment explaining your approach helps avoid duplicated effort.

**Have an idea for a new module?** The module system is the right home for new subsystems. If you've built something useful on top of WolfEngine — an animation system, a tween manager, a save format extension — it probably belongs there. Feel free to share the idea first if you're unsure whether it fits.

**Improving existing modules or components?** Performance fixes, correctness improvements, and better lifecycle hook coverage are all welcome. If you're touching the collision or save system, make sure the module still compiles cleanly with its macro both enabled and disabled.

**Documentation?** Corrections, clearer explanations, and honest notes about current limitations are just as valuable as code.

---

## Contribution Guidelines

### Adding a Module

New subsystems belong in the module system — not as core engine members. If you're adding one:

1. Create your module class under `Modules/` and name it with the `WE_` prefix (e.g. `WE_YourModule`).
2. Implement `IModule` and override only the lifecycle hooks your module actually needs.
3. Declare a static instance in `WE_ModuleSystem.cpp` and add it to `s_modules[]` under a new macro guard (e.g. `WE_MODULE_YOURFEATURE`).
4. Add that macro to `WE_Settings.hpp` with a sensible default.
5. If your module has ordering dependencies, set its priority accordingly — `InitAll()` sorts by descending priority before calling init.
6. Confirm the build works with your macro both enabled and disabled.

### Adding a Component

1. Inherit from `Component` (`WE_BaseComp.hpp`).
2. Enable only the tick phases your component genuinely uses via the tick flags: `earlyTickEnabled`, `tickEnabled`, `lateTickEnabled`, `preRenderTickEnabled`.
3. Register colliders through `WE_CollisionModule::Get()` — not through `Engine().m_ColliderManager`, which no longer exists.

> **Note:** `preRenderComponentTick()` currently calls `preRenderTick()` without checking `preRenderTickEnabled`. Keep this in mind until it's resolved.

---

## Code Style

- **Language standard:** C++20. No exceptions.
- **Naming:** `WE_` prefix for engine types. `m_` prefix for member variables. `s_` prefix for statics.
- **Heap:** Don't introduce new heap allocation casually. Prefer stack allocation and static instances for module-level objects.
- **STL:** Not blanket-forbidden. Check what the surrounding code already uses before assuming it's off-limits.
- **Feature flags:** New features that should be opt-in go behind a macro in `WE_Settings.hpp`, following the `WE_MODULE_*` pattern. Code that depends on the flag must compile cleanly both ways.
- **Accessors:** Use the provided global accessors (`Input()`, `Sound()`, `UI()`, `MainCamera()`, `RenderSys()`) rather than reaching into the engine singleton directly.

---

## Submitting Changes

### PR Hygiene

- One logical change per PR. If you're fixing two unrelated things, open two PRs.
- Keep commits clean and descriptive. Squash fixup commits before requesting review.
- If a PR touches the module system, confirm it builds with the relevant macro both on and off.
- If a PR changes phase behavior or game loop timing, document what changed and why it's safe.

### PR Template

Use this format when opening a pull request:

---

```
## <Short Title>

**Type:** Bug Fix | New Module | New Component | Refactor | Docs
**Scope:** <file(s) or subsystem affected>
**What changed:** <what you did and why>

**Tested:**
- [ ] ESP32 hardware
- [ ] Desktop

**Possible additions:** *(optional) follow-up work or extensions this opens up*
**Possible problems:** *(optional) risks, edge cases, or things reviewers should watch for*
```
