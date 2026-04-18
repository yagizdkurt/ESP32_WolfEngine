# Transform Component

The Transform Component holds a [GameObject](gameobject.md)'s position and size in world space. It is the only component that is **automatically present on every GameObject** — you never need to add it manually.

---

## Accessing Transform

Transform is a public member of every GameObject, accessible directly:

```cpp
void Update() override {
    transform.position.x += 1;  // move right
}
```

---

## Fields

### `position` — `Vec2`
The object's position in world space. Default is `(0, 0)`.

```cpp
transform.position = { 100, 50 };
transform.position.x += 1;
transform.position.y -= 1;
```

### `width` — `uint8_t`
The object's width in pixels. Default is `1`.

### `height` — `uint8_t`
The object's height in pixels. Default is `1`.

```cpp
transform.width  = 8;
transform.height = 8;
```

---

## Notes

- Transform values can be read and written freely from anywhere that has access to the GameObject.
