# Camera

The camera defines what part of the game world is visible on screen. It supports free positioning, zoom, smooth target following, and coordinate conversion between world and screen space.

The camera is owned by WolfEngine and accessed anywhere in game code through the global `MainCamera()` function.

---

## Accessing the Camera

```cpp
MainCamera().setPosition({ 0, 0 });
MainCamera().setZoom(2.0f);
```

---

## Position

### `setPosition(Vec2 pos)`
Instantly moves the camera to the given world position.

```cpp
MainCamera().setPosition({ 100, 50 });
```

### `getPosition()`
Returns the current camera position as a `Vec2`.

```cpp
Vec2 pos = MainCamera().getPosition();
```

### `move(Vec2 delta)`
Moves the camera by the given offset relative to its current position.

```cpp
MainCamera().move({ 1, 0 });  // scroll right by 1 unit
```

---

## Zoom

### `setZoom(float zoom)`
Sets the zoom level directly. `1.0` is default, values above zoom in, values below zoom out. Minimum zoom is `0.1`.

```cpp
MainCamera().setZoom(2.0f);  // 2x zoom in
```

### `getZoom()`
Returns the current zoom level.

### `zoomIn(float amount)` / `zoomOut(float amount)`
Adjusts zoom by the given amount relative to current zoom.

```cpp
MainCamera().zoomIn(0.5f);
MainCamera().zoomOut(0.5f);
```

### `zoomReset()`
Resets zoom back to `1.0`.

---

## Target Following

The camera can follow a `GameObject` automatically every frame, either snapping directly or smoothly interpolating toward it.

### `setTarget(GameObject* target, float speed = 0.1f)`
Sets a GameObject for the camera to follow. `speed` controls how quickly the camera catches up — `0.1` is a gentle follow, `1.0` snaps immediately. Called once in `Start()` typically.

```cpp
void Start() override {
    MainCamera().setTarget(this);          // follow this object, default speed
    MainCamera().setTarget(this, 0.05f);   // slower, smoother follow
    MainCamera().setTarget(this, 1.0f);    // instant snap
}
```

### `clearTarget()`
Stops following any target. The camera stays at its current position.

```cpp
MainCamera().clearTarget();
```

---

## Coordinate Conversion

### `worldToScreen(Vec2 world_pos)`
Converts a world space position to screen pixel coordinates, taking camera position and zoom into account. Useful for drawing UI elements that track world objects.

```cpp
Vec2 screenPos = MainCamera().worldToScreen(enemy.transform.position);
```

### `screenToWorld(Vec2 screen_pos)`
Converts a screen pixel position back to world space. Useful for touch or cursor input.

```cpp
Vec2 worldPos = MainCamera().screenToWorld({ 64, 80 });
```

### `isVisible(Vec2 world_pos, float margin = 0.0f)`
Returns `true` if the world position is currently visible on screen. Optional `margin` extends the visible area beyond screen edges — useful for culling checks where you want to keep objects alive slightly off-screen.

```cpp
if (MainCamera().isVisible(enemy.transform.position, 8.0f)) {
    enemy.draw();
}
```

---

## Reset

### `reset()`
Resets position to `(0, 0)`, zoom to `1.0`, and clears any follow target.

```cpp
MainCamera().reset();
```
