# Input

WolfEngine's input system handles button polling and analog joystick reading with built-in software debouncing. It supports up to 4 controllers simultaneously, each with buttons wired directly to GPIO pins, buttons routed through an I2C expander (PCF8574, PCF8575, or MCP23017), and an analog joystick via ADC — all configured from a single settings file.

---

## Accessing Input

Input is available anywhere in game code through the global `Input()` function. First get a reference to the controller you want to query, then call methods on it:

```cpp
Controller* controller = nullptr;

void Start() override {
    controller = Input().getController(0); // player 1
}

void Update() override {
    if (controller && controller->getButtonDown(Button::A)) {
        // do something
    }
}
```

> **Note:** Always store the controller pointer in `Start()` rather than calling `getController()` every frame.

---

## Controllers

WolfEngine supports up to 4 simultaneous controllers. Each controller is configured independently in [Input Settings](../input.md).

```cpp
Controller* p1 = Input().getController(0); // player 1
Controller* p2 = Input().getController(1); // player 2
```

`getController()` expects indices in the `0..3` range. Passing an out-of-range index will trigger a debug assert. It returns `nullptr` for disabled controller slots. Always null-check before use if unsure:

```cpp
Controller* p2 = Input().getController(1);
if (p2) {
    p2->getButtonDown(Button::A);
}
```

---

## Buttons

WolfEngine supports 10 buttons per controller:

| Button | Enum |
|--------|------|
| A | `Button::A` |
| B | `Button::B` |
| C | `Button::C` |
| D | `Button::D` |
| E | `Button::E` |
| F | `Button::F` |
| G | `Button::G` |
| H | `Button::H` |
| I | `Button::I` |
| J | `Button::J` |

Each button can be wired to a direct GPIO pin, an expander pin, or left unassigned — all configured in [Input Settings](../input.md).

> **Note:** Button labels have no predefined meaning in the engine. They are just identifiers. Your controller may have ABXY, directions, or any other layout — wire them however you like and assign meaning in your game code.

---

## Button Queries

### `getButton(Button btn)`
Returns `true` every frame the button is held down.
```cpp
if (controller && controller->getButton(Button::B)) {
    player.accelerate();
}
```

### `getButtonDown(Button btn)`
Returns `true` only on the single frame the button is first pressed. Use this for one-shot actions.
```cpp
if (controller && controller->getButtonDown(Button::A)) {
    player.jump();
}
```

### `getButtonUp(Button btn)`
Returns `true` only on the single frame the button is released.
```cpp
if (controller && controller->getButtonUp(Button::A)) {
    player.stopCharging();
}
```

---

## Analog Joystick

The joystick returns a normalized float value per axis in the range **-1.0 to +1.0**. Values inside the dead zone return exactly **0.0** to prevent stick drift.

```cpp
float x = controller->getAxis(JoyAxis::X);
float y = controller->getAxis(JoyAxis::Y);
```

| Value | Meaning |
|-------|---------|
| `-1.0` | Full negative (left / down) |
| `0.0` | Center or inside dead zone |
| `+1.0` | Full positive (right / up) |

Dead zone size and axis calibration are configured per controller in [Input Settings](../input.md). If a joystick axis is disabled, `getAxis()` always returns `0.0`.

---

## Supported Expanders

Buttons can be routed through any of the following I2C expanders instead of direct GPIO:

| Chip | Type | Pins | Address Range |
|------|------|------|---------------|
| PCF8574 | 8-bit, no registers | 0–7 | 0x20–0x27 |
| PCF8575 | 16-bit, no registers | 0–15 | 0x20–0x27 |
| MCP23017 | 16-bit, register-based | 0–15 | 0x20–0x27 |

Expander type and address are configured per controller in [Input Settings](../input.md).

---

## Debouncing

All buttons across all controllers are automatically debounced in software. A button state change is only committed after the signal has remained stable for the duration defined by `debounceMs` in [Input Settings](../input.md). This prevents noise and contact bounce from registering false presses. Debounce timing is engine-wide and applies equally to all controllers.

---

## Configuration

All input routing, expander setup, joystick calibration, and controller enabling is done in `WE_Settings.hpp`. See [Input Settings](../input.md) for the full reference.
