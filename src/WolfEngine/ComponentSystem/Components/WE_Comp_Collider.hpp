#pragma once
#include "WolfEngine/Settings/WE_Settings.hpp"
#if defined(WE_MODULE_COLLISION)

#include <stdint.h>
#include "WE_BaseComp.hpp"
#include "WE_Comp_SpriteRenderer.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"

class WE_CollisionModule;
enum class ColliderShape : uint8_t { Box, Circle };

/**
 * # Collider  Component
 *
 * Tracks box and circle collision bounds for an attached GameObject.
 *
 * Registers each instance with the collision module, stores layer and mask
 * filtering data, and resolves world-space bounds from the owner transform
 * plus local offset.
 *
 * ### Notes:
 * - Requires `WE_MODULE_COLLISION`.
 * - Create instances with `Box()` or `Circle()`; the constructor is private.
 */
class Collider : public Component {
public:
    /** ## Creates a box collider with explicit local dimensions.
     *
     *  @param owner Owning GameObject; non-owning and expected to outlive the collider.
     *  @param w Local box width.
     *  @param h Local box height.
     *  @param layer Collision layer used for filtering.
     *  @param mask Bitmask of layers this collider can interact with.
     *  @param trigger True to route overlaps through trigger callbacks.
     *  @return Box collider configured with the supplied bounds.
     */
    static Collider Box(GameObject* owner, int w, int h, CollisionLayer layer = CollisionLayer::DEFAULT, uint16_t mask = 0xFFFF, bool trigger = false);
    /** ## Creates a box collider that uses the owner visual-fit path.
     *
     *  @param owner Owning GameObject; non-owning and expected to outlive the collider.
     *  @param layer Collision layer used for filtering.
     *  @param mask Bitmask of layers this collider can interact with.
     *  @param trigger True to route overlaps through trigger callbacks.
     *  @return Box collider prepared for visual-bound alignment.
     */
    static Collider Box(GameObject* owner, CollisionLayer layer = CollisionLayer::DEFAULT, uint16_t mask = 0xFFFF, bool trigger = false);
    /** ## Creates a circle collider with an explicit radius.
     *
     *  @param owner Owning GameObject; non-owning and expected to outlive the collider.
     *  @param radius Local circle radius.
     *  @param layer Collision layer used for filtering.
     *  @param mask Bitmask of layers this collider can interact with.
     *  @param trigger True to route overlaps through trigger callbacks.
     *  @return Circle collider configured with the supplied radius.
     */
    static Collider Circle(GameObject* owner, int radius, CollisionLayer layer = CollisionLayer::DEFAULT, uint16_t mask = 0xFFFF, bool trigger = false);
    /** ## Destroys the collider and unregisters it from the collision module. */
    ~Collider();

    void setMask   (uint16_t mask)        { m_mask      = mask;    }
    void setLayer  (CollisionLayer layer) { m_layer     = layer;   }
    void setTrigger(bool trigger)         { m_isTrigger = trigger; }
    void setOffset (float x, float y)     { m_offsetX   = x; m_offsetY = y; }
    /** ## Sets the collider's box dimensions.
     *
     *  @param width Local box width.
     *  @param height Local box height.
     */
    void setSize   (int width, int height);
    /** ## Sets the collider's circle radius.
     *
     *  @param radius Local circle radius.
     */
    void setRadius (int radius);
    /** ## Binds a sprite renderer for visual-bound alignment.
     *
     *  @param sr Non-owning sprite renderer pointer, or null to clear the binding.
     */
    void bindSpriteRenderer(const SpriteRenderer* sr) { m_spriteRenderer = sr; fitToOwnerVisuals(); }
    /** ## Prepares collider bounds from the bound sprite renderer.
     *
     *  The collider shape must already match the visual source.
     */
    void fitToOwnerVisuals();

    GameObject*    getOwner()   const { return m_owner;                }
    ColliderShape  getShape()   const { return m_shape;                }
    CollisionLayer getLayer()   const { return m_layer;                }
    uint16_t       getMask()    const { return m_mask;                 }
    bool           isTrigger()  const { return m_isTrigger;            }
    float          getOffsetX() const { return m_offsetX;              }
    float          getOffsetY() const { return m_offsetY;              }
    int            getWidth()   const { return m_bounds.box.width;     }
    int            getHeight()  const { return m_bounds.box.height;    }
    int            getRadius()  const { return m_bounds.circle.radius; }
    /** ## Returns the collider world X coordinate.
     *
     *  @return Owner transform X plus the collider's local X offset.
     */
    float          getWorldX()  const;
    /** ## Returns the collider world Y coordinate.
     *
     *  @return Owner transform Y plus the collider's local Y offset.
     */
    float          getWorldY()  const;

private:
    Collider(GameObject* owner, ColliderShape shape, CollisionLayer layer, uint16_t mask, bool trigger);

    GameObject*           m_owner          = nullptr;
    const SpriteRenderer* m_spriteRenderer = nullptr;
    ColliderShape         m_shape;
    CollisionLayer        m_layer          = CollisionLayer::DEFAULT;
    uint16_t              m_mask           = 0xFFFF;
    bool                  m_isTrigger      = false;
    float                 m_offsetX        = 0.0f;
    float                 m_offsetY        = 0.0f;
    int                   m_slot           = -1;

    union {
        struct { int width; int height; } box;
        struct { int radius;            } circle;
    } m_bounds = { {1, 1} };

    friend class WE_CollisionModule;
    void OnCollisionEnter(Collider* other) { if (m_owner) m_owner->OnCollisionEnter(other); }
    void OnCollisionStay (Collider* other) { if (m_owner) m_owner->OnCollisionStay(other);  }
    void OnCollisionExit (Collider* other) { if (m_owner) m_owner->OnCollisionExit(other);  }
    void OnTriggerEnter  (Collider* other) { if (m_owner) m_owner->OnTriggerEnter(other);   }
    void OnTriggerStay   (Collider* other) { if (m_owner) m_owner->OnTriggerStay(other);    }
    void OnTriggerExit   (Collider* other) { if (m_owner) m_owner->OnTriggerExit(other);    }
};

#else
    template<bool Enabled = false> struct ColliderDisabled {
    static_assert(Enabled, "Collider is disabled — enable WE_MODULE_COLLISION in WE_Settings.hpp to use it");};
    using Collider = ColliderDisabled<>;
#endif // WE_MODULE_COLLISION