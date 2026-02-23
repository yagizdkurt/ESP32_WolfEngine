#pragma once
#include <stdint.h>
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/Graphics/ColorPalettes/WE_Palette_Grayscale.hpp"

// =============================================================
//  WE_BaseUIElement
//  Abstract base class for all UI elements.
//  UITransform is stored in flash (const) — position never changes.
//  visible, dirty, color index, and palette pointer live in RAM.
//
//  Never instantiate directly — use UILabel, UIBar, etc.
// =============================================================

class UIManager;  // forward declaration

// --- Stored in flash (const) ---
struct UITransform {
    int16_t x;       // screen x position
    int16_t y;       // screen y position (see anchor)
    bool    anchor;  // if true, y is relative to RENDER_UI_START_ROW
                     // e.g. anchor=true, y=12 → draws at y=140 (128+12)
};

// =============================================================
// THIS NEEDS TO BE NULL TERMINATED!!! PLEASE DONT FORGET THIS
// =============================================================
class BaseUIElement {
public:
    BaseUIElement(const UITransform* transform)
        : m_transform(transform)
        , m_manager(nullptr)
        , m_visible(true)
        , m_dirty(false) {}

    virtual ~BaseUIElement() = default;
    virtual void draw(class UIManager& mgr) = 0;  // pure virtual — must implement

    void show()  { m_visible = true;  markDirty(); }
    void hide()  { m_visible = false; markDirty(); }

    bool isVisible() const { return m_visible; }
    bool isDirty()   const { return m_dirty;   }
    void clearDirty()      { m_dirty = false;  }

    // Resolves y coordinate — applies anchor offset if set
    int16_t getX() const { return m_transform->x; }
    int16_t getY() const {
        return m_transform->anchor
            ? m_transform->y + RENDER_UI_START_ROW
            : m_transform->y;
    }

protected:
    const UITransform* m_transform;
    UIManager*         m_manager;
    bool               m_visible;
    bool               m_dirty;

    void markDirty();  // implemented in WE_BaseUIElement.cpp

    friend class UIManager;
};
