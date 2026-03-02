#pragma once
#include <stdint.h>
#include "WE_BaseComp.hpp"
#include "WE_Comp_SpriteRenderer.hpp"
#include "WolfEngine/GameObjectSystem/WE_GameObject.hpp"
#include "WolfEngine/Settings/WE_Layers.hpp"

class ColliderManager;
enum class ColliderShape : uint8_t { Box, Circle };

class Collider : public Component {
public:
    static Collider Box(GameObject* owner, int w, int h, CollisionLayer layer = LAYER_DEFAULT, uint16_t mask = 0xFFFF, bool trigger = false);
    static Collider Box(GameObject* owner, CollisionLayer layer = LAYER_DEFAULT, uint16_t mask = 0xFFFF, bool trigger = false);
    static Collider Circle(GameObject* owner, int radius, CollisionLayer layer = LAYER_DEFAULT, uint16_t mask = 0xFFFF, bool trigger = false);
    ~Collider();

    void setMask   (uint16_t mask)        { m_mask      = mask;    }
    void setLayer  (CollisionLayer layer) { m_layer     = layer;   }
    void setTrigger(bool trigger)         { m_isTrigger = trigger; }
    void setOffset (float x, float y)     { m_offsetX   = x; m_offsetY = y; }
    void setSize   (int width, int height);
    void setRadius (int radius);
    void bindSpriteRenderer(const SpriteRenderer* sr) { m_spriteRenderer = sr; fitToOwnerVisuals(); }
    void fitToOwnerVisuals();

    GameObject*      getOwner()   const { return m_owner;               }
    ColliderShape    getShape()   const { return m_shape;               }
    CollisionLayer   getLayer()   const { return m_layer;               }
    uint16_t         getMask()    const { return m_mask;                }
    bool             isTrigger()  const { return m_isTrigger;           }
    float            getOffsetX() const { return m_offsetX;             }
    float            getOffsetY() const { return m_offsetY;             }
    ColliderManager* getManager() const { return m_manager;             }
    int              getWidth()   const { return m_bounds.box.width;    }
    int              getHeight()  const { return m_bounds.box.height;   }
    int              getRadius()  const { return m_bounds.circle.radius;}
    float            getWorldX()  const;
    float            getWorldY()  const;

private:
    Collider(GameObject* owner, ColliderShape shape, CollisionLayer layer, uint16_t mask, bool trigger);

    GameObject*           m_owner          = nullptr;
    const SpriteRenderer* m_spriteRenderer = nullptr;
    ColliderShape         m_shape;
    CollisionLayer        m_layer          = LAYER_DEFAULT;
    uint16_t              m_mask           = 0xFFFF;
    bool                  m_isTrigger      = false;
    float                 m_offsetX        = 0.0f;
    float                 m_offsetY        = 0.0f;
    ColliderManager*      m_manager        = nullptr;

    union {
        struct { int width; int height; } box;
        struct { int radius;            } circle;
    } m_bounds = { {1, 1} };

    friend class ColliderManager;
    
    void OnCollisionEnter(Collider* other) { if (m_owner) m_owner->OnCollisionEnter(other); }
    void OnCollisionStay (Collider* other) { if (m_owner) m_owner->OnCollisionStay(other);  }
    void OnCollisionExit (Collider* other) { if (m_owner) m_owner->OnCollisionExit(other);  }
    void OnTriggerEnter  (Collider* other) { if (m_owner) m_owner->OnTriggerEnter(other);   }
    void OnTriggerStay   (Collider* other) { if (m_owner) m_owner->OnTriggerStay(other);    }
    void OnTriggerExit   (Collider* other) { if (m_owner) m_owner->OnTriggerExit(other);    }
};
