#pragma once
#include <cstddef>
#include "WolfEngine/Graphics/UserInterface/UIElements/Base/WE_BaseUIElement.hpp"

// Non-null UI element handle used by UIManager/UIPanel registration APIs.
// Constructing from pointers (including nullptr) is intentionally forbidden.
class UIElementRef {
public:
    constexpr UIElementRef(BaseUIElement& element) : m_ptr(&element) {}
    UIElementRef(BaseUIElement*) = delete;
    UIElementRef(std::nullptr_t) = delete;

    constexpr BaseUIElement* get() const { return m_ptr; }

private:
    BaseUIElement* m_ptr;
};
