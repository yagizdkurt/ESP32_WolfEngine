#include "WE_BaseUIElement.hpp"
#include "WolfEngine/Graphics/UserInterface/WE_UIManager.hpp"

void BaseUIElement::show()      { m_visible = true;  markDirty(); }
void BaseUIElement::hide()      { m_visible = false; markDirty(); }
void BaseUIElement::markDirty() { if (m_manager) m_manager->m_dirty = true; }
