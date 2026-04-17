#pragma once

// -------------------------------------------------------------
//  Identifies which I/O expander chip is used by a controller.
//  Set to None if all buttons are wired directly to GPIO.
// -------------------------------------------------------------
enum class ExpanderType {
    None,       // No expander — all buttons on direct GPIO
    PCF8574,    // 8-bit expander,  no registers, I2C
    PCF8575,    // 16-bit expander, no registers, I2C
    MCP23017    // 16-bit expander, register-based, I2C
};

// -------------------------------------------------------------
//  Configuration for one I/O expander attached to a controller.
//
//  type  — which chip is used, or ExpanderType::None to disable
//  addr  — 7-bit I2C address of the chip (e.g. 0x20)
//  pins  — maps each button index to an expander pin (0–7 for
//          8-bit chips, 0–15 for 16-bit chips). Set to -1 if
//          the button is not wired through the expander.
// -------------------------------------------------------------
struct ExpanderSettings {
    ExpanderType type;
    int          addr;
    int          pins[10]; // one entry per button, matches BUTTON_COUNT
};