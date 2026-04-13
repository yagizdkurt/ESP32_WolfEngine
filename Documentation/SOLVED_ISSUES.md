# Resolved Issues
This file is a permanent record of known issues that have been identified and resolved
during development. It is append-only — entries are never deleted or edited after they
are written. Active (unresolved) issues live in KNOWN_ISSUES.md.

## Entry Format
Each entry follows this exact structure:

## <Short Title>
RESOLVED in <Phase> - <DD.MM.YY>
**Location:** <file(s) and specific area affected>
**What it did:** <what the problem was and why it was acceptable or unnoticed>
**Resolution (<Phase>):** <what was changed and why, including any design decisions>
- Bullet points for specific code or file changes
- Keep it precise enough that a future developer can understand the decision without
  reading the git diff
**Extra Notes** any extra notes to be informed.
---

## Rules for AI Assistants
- When resolving an entry from KNOWN_ISSUES.md, move it here — do not delete it.
- Strike-through in KNOWN_ISSUES.md is not sufficient — the full resolved entry
  belongs here with a proper Resolution section added.
- Never edit existing entries in this file. Add new entries only at the bottom.
- Always include the phase and date in the RESOLVED line.
- The Resolution section must explain the mechanism, not just say "fixed".
- If multiple issues are resolved in the same phase, add them as separate entries.

-----------------------------------------------------------------------------------------

## Shutdown Signal is Hard Kill
RESOLVED in Phase 2 - 13.04.26
**Location:** desktop/main_desktop.cpp — ESC/window-close handler
**What it did:** called std::exit(0) which hard-killed the process. The detached engine
thread got no chance to run destructors or release resources. Acceptable in Phase 1 because
the engine held no real resources.
**Resolution (Phase 2):** `WolfEngine` gains a proper quit mechanism — no #ifdefs, no
globals, no desktop-specific code in engine source:
- `m_isRunning` promoted from `bool` to `std::atomic<bool>` inside `WolfEngine`.
- `WolfEngine::RequestQuit()` stores `false` into `m_isRunning` (callable from any thread).
- `WolfEngine::IsRunning()` loads `m_isRunning` (callable from any thread).
- `StartGame()` loop changes from `while (true)` to `while (IsRunning())`.
- `main_desktop.cpp` calls `Engine().RequestQuit()` on ESC/quit, then
  `engineThread.join()`, then destroys SDL objects in order before returning normally.
- `std::exit(0)` is removed entirely.