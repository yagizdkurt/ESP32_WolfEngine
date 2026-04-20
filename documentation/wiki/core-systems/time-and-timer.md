# Time and Timer

WolfEngine provides two timing utilities:

- `WETime`: global static time helpers
- `Timer`: per-instance timer wrapper for gameplay code

Paused duration is excluded from game-time calculations.

---

## Timer

`Timer` wraps a timestamp and active flag.

```cpp
class Player : public GameObject {
    Timer m_shootCooldown;

    void Start() override {
        m_shootCooldown.start();
    }

    void Update() override {
        if (m_shootCooldown.check(500)) {
            shoot();
        }
    }
};
```

### Lifecycle

```cpp
timer.start(); // activate and reset to now
timer.stop();  // deactivate
timer.reset(); // reset timestamp, keep current active state
```

### Queries

```cpp
timer.elapsed(500); // true once 500ms has passed (no reset)
timer.check(500);   // true when 500ms has passed, then advances timestamp by duration
timer.timeout(500); // true while 500ms has not passed yet
timer.isActive();   // whether timer is active
```

`elapsed`, `check`, and `timeout` return `false` while inactive.

---

## WETime

`WETime` is static-only and does not require an instance.

### Delta Time

```cpp
float dt = WETime::DELTA_TIME;
```

`DELTA_TIME` is a compile-time value derived from `TARGET_FRAME_TIME_US`.

### Time Accessors

```cpp
int64_t msGame = WETime::now();
int64_t usGame = WETime::nowUs();

int64_t msReal = WETime::realNow();
int64_t usReal = WETime::realNowUs();
```

- `now` / `nowUs`: game time (pause-adjusted)
- `realNow` / `realNowUs`: wall-clock time from `esp_timer_get_time()`

### Since Helpers

```cpp
int64_t start = WETime::now();
int64_t aliveMs = WETime::since(start);
int64_t aliveUs = WETime::sinceUs(start);
```

### Pause and Resume

```cpp
WETime::pause();
WETime::resume();
bool paused = WETime::isPaused();
```

### Frame Counter

```cpp
uint32_t frame = WETime::frameCount();
```

Frame count is incremented by the engine tick path.

---

## WETime vs Timer

| Feature | `WETime` | `Timer` |
|---|---|---|
| State ownership | Global static | Per instance |
| Active/inactive state | No | Yes |
| Convenience checks | Manual math via timestamps | Built-in `elapsed/check/timeout` |
| Typical use | Shared or ad-hoc time reads | Cooldowns and object-local timers |
