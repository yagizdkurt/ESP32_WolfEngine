#pragma once
#include <stdint.h>
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/Graphics/ColorPalettes/WE_Palette_Grayscale.hpp"
class UIManager;


// --- Stored in flash (const) ---
// { X,Y,Anchor }
// If anchor=true, Y is treated as an offset from the bottom of the screen.
struct UITransform { int16_t x; int16_t y; bool anchor; };

// =============================================================
// THIS NEEDS TO BE NULL TERMINATED!!! PLEASE DONT FORGET THIS
// =============================================================
//  WE_BaseUIElement
//  Abstract base class for all UI elements.
//  UITransform is stored in flash (const) - position never changes.
//  visible, dirty, color index, and palette pointer live in RAM.
//
//  Never instantiate directly - use UILabel, UIBar, etc.
// =============================================================
class BaseUIElement {
public:
    // Bind this element to a fixed transform.
    BaseUIElement(const UITransform* transform);

    virtual ~BaseUIElement() = default;
    // Render this element when dirty and visible.
    virtual void draw(class UIManager& mgr) = 0;  // pure virtual - must implement

    // Make the element visible and mark UI dirty.
    void show();
    // Hide the element and mark UI dirty.
    void hide();

    // Return current visibility state.
    bool isVisible() const;
    // Return whether this element needs redraw.
    bool isDirty() const;
    // Clear dirty flag after rendering.
    void clearDirty();

    // Get absolute X coordinate.
    int16_t getX() const;
    // Get absolute Y coordinate (applies anchor offset when enabled).
    int16_t getY() const;

protected:
    const UITransform* m_transform;
    UIManager*         m_manager;
    bool               m_visible;
    bool               m_dirty;

    void markDirty();  // implemented in WE_BaseUIElement.cpp

    friend class UIManager;
};
