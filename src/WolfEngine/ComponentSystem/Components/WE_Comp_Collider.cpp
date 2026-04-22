#include "WE_Comp_Collider.hpp"
#if defined(WE_MODULE_COLLISION)

#include "WolfEngine/Modules/CollisionSystem/WE_CollisionModule.hpp"

// Private constructor
Collider::Collider(GameObject* owner, ColliderShape shape, CollisionLayer layer, uint16_t mask, bool trigger)
    : m_owner    (owner)
    , m_shape    (shape)
    , m_layer    (layer)
    , m_mask     (mask)
    , m_isTrigger(trigger)
{
    type        = ComponentType::COMP_COLLIDER;
    tickEnabled = false;
    owner->registerComponent(this);
    WE_CollisionModule::Get().RegisterCollider(this);
}

// Factory - Box with manual size
Collider Collider::Box(GameObject* owner, int w, int h, CollisionLayer layer, uint16_t mask, bool trigger) {
    Collider c(owner, ColliderShape::Box, layer, mask, trigger);
    c.m_bounds.box.width  = w;
    c.m_bounds.box.height = h;
    return c;
}

// Factory - Box auto-fit to SpriteRenderer
Collider Collider::Box(GameObject* owner, CollisionLayer layer, uint16_t mask, bool trigger) {
    Collider c(owner, ColliderShape::Box, layer, mask, trigger);
    c.fitToOwnerVisuals();
    return c;
}

// Factory - Circle
Collider Collider::Circle(GameObject* owner, int radius, CollisionLayer layer, uint16_t mask, bool trigger) {
    Collider c(owner, ColliderShape::Circle, layer, mask, trigger);
    c.m_bounds.circle.radius = radius;
    return c;
}

// Destructor
Collider::~Collider() {
    WE_CollisionModule::Get().UnregisterCollider(this);
}

// setSize / setRadius
void Collider::setSize(int width, int height) {
    m_bounds.box.width  = width;
    m_bounds.box.height = height;
}

void Collider::setRadius(int radius) {
    m_bounds.circle.radius = radius;
}

// fitToOwnerVisuals TODO: make a get shape function in SpriteRenderer and use it here to determine the collider shape
void Collider::fitToOwnerVisuals() {
    if (m_spriteRenderer == nullptr) return;

    switch (m_shape) {
        case ColliderShape::Box:

            break;
        case ColliderShape::Circle:

            break;
    }

}

// getWorldX / getWorldY - transform position + offset
float Collider::getWorldX() const { return m_owner->transform.position.x + m_offsetX; }
float Collider::getWorldY() const { return m_owner->transform.position.y + m_offsetY; }

#endif // WE_MODULE_COLLISION
