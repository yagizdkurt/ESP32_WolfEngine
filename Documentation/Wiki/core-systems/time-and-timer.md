# Time & Timer

WolfEngine provides two time utilities — `WETime` for global static time access and `Timer` for convenient per-object timing. Both automatically exclude paused duration from all calculations.

---

## Timer

`Timer` is a convenience struct that wraps an `int64_t` timestamp and calls `WETime::` internally. Use it when you want cleaner game code without managing raw timestamps manually.

```cpp
class Player : public GameObject {
    Timer m_shootCooldown;
    Timer m_invincibility;

    void Start() override {
        m_shootCooldown.start();
        m_invincibility.start();
    }

    void Update() override {
        if (m_shootCooldown.check(500)) {
            shoot();
        }

        if (m_invincibility.timeout(2000)) {
            renderInvincibilityEffect();
        }
    }
};
```

### Lifecycle

```cpp
timer.start(); // activate and reset timestamp to now
timer.stop();  // deactivate — all queries return false
timer.reset(); // reset timestamp without changing active state
```

A `Timer` must be started with `start()` before any queries return meaningful results. All queries return `false` when inactive.

### Queries

```cpp
timer.elapsed(500)  // has 500ms passed? no auto-reset
timer.check(500)    // has 500ms passed? auto-resets if true
timer.timeout(500)  // has 500ms NOT yet passed?
timer.isActive()    // is the timer running?
```

---

## WETime

`WETime` is a static utility class — no instance needed, call methods directly:

```cpp
WETime::now();
WETime::elapsed(m_timestamp, 500);
```

### Delta Time

A compile time constant calculated from your target framerate. Use it for frame-rate independent movement:

```cpp
transform.position.x += speed * WETime::DELTA_TIME;
```

Changing `TARGET_FRAME_TIME_US` in settings automatically updates `DELTA_TIME`.

### Current Time

```cpp
int64_t ms = WETime::now();    // game time in milliseconds
int64_t us = WETime::nowUs();  // game time in microseconds

int64_t ms = WETime::realNow();    // real time in milliseconds — not affected by pause
int64_t us = WETime::realNowUs();  // real time in microseconds — not affected by pause
```

### Elapsed

Returns true if `durationMs` has passed since the timer was set. Does not auto-reset:

```cpp
int64_t m_timestamp = 0;

void Start() override {
    m_timestamp = WETime::now();
}

void Update() override {
    if (WETime::elapsed(m_timestamp, 1000)) {
        // 1 second has passed
    }
}
```

### Check

Returns true if `durationMs` has passed and automatically resets the timer. Ideal for cooldowns and repeating events:

```cpp
if (WETime::check(m_shootCooldown, 500)) {
    shoot(); // fires at most once every 500ms
}
```

### Timeout

Returns true while `durationMs` has NOT yet passed. Useful for showing something for a fixed duration:

```cpp
if (WETime::timeout(m_showTimer, 2000)) {
    renderMessage(); // visible for 2 seconds
}
```

### Reset

Manually sets a timer to current game time:

```cpp
WETime::reset(m_timestamp);
```

### Since

How much time has passed since a recorded timestamp:

```cpp
int64_t spawnTime = WETime::now();
int64_t aliveMs   = WETime::since(spawnTime);
int64_t aliveUs   = WETime::sinceUs(spawnTime);
```

### Frame Count

Total game frames elapsed since start. Does not increment while paused:

```cpp
uint32_t frame = WETime::frameCount();

// trigger something every 30 frames
if (WETime::frameCount() % 30 == 0) { ... }
```

### Pause & Resume

Freezes game time. All `elapsed`, `check`, `timeout`, and `since` calculations automatically exclude paused duration — no manual adjustment needed:

```cpp
WETime::pause();
WETime::resume();
bool paused = WETime::isPaused();
```

> **Note:** `realNow()` and `realNowUs()` are not affected by pause — they always return true elapsed time since boot.

---

### WETime:: vs Timer

|                | `WETime::`                | `Timer`                  |
|----------------|---------------------------|--------------------------|
| Storage        | `int64_t` member variable | `Timer` member variable  |
| Reset          | `WETime::reset(m_timer)`  | `m_timer.reset()`        |
| Check          | `WETime::check(m_timer, 500)` | `m_timer.check(500)` |
| Active state   | No                        | Yes — `start()` / `stop()` |
| Best for       | Simple one-off timestamps | Cooldowns, repeating events |

### Frame Duration Reference

At 30fps one frame = ~33ms. Useful for thinking about timer durations:

| Duration | Frames at 30fps |
|----------|-----------------|
| 33ms     | 1 frame         |
| 100ms    | 3 frames        |
| 500ms    | 15 frames       |
| 1000ms   | 30 frames       |
| 2000ms   | 60 frames       |
