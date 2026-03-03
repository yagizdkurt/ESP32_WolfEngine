#include "WE_ColliderManager.hpp"
#include "WolfEngine/ComponentSystem/Components/WE_Comp_Collider.hpp"
#include <algorithm>

ColliderManager::ColliderManager() noexcept {
    for (int i = 0; i < MAX_COLLIDERS; ++i) { m_freeStack[i] = i; }
    m_freeTop = MAX_COLLIDERS;
}


void ColliderManager::registerCollider(Collider* collider) {
    if (m_freeTop == 0) return;
    const int slot    = m_freeStack[--m_freeTop];
    m_colliders[slot] = collider;
    collider->m_slot  = slot;
}

void ColliderManager::unregisterCollider(Collider* collider) {
    const int slot = collider->m_slot;
    if (slot < 0 || slot >= MAX_COLLIDERS) return;      // not registered
    m_colliders[slot] = nullptr;
    collider->m_slot  = -1;
    clearPairsForSlot(slot);
    m_freeStack[m_freeTop++] = slot;                    // return slot to free-list
}

void ColliderManager::tick() {
    clearBits(m_currPairs);

    for (int i = 0; i < MAX_COLLIDERS; ++i) {
        Collider* a = m_colliders[i];
        if (a == nullptr) continue;

        for (int j = i + 1; j < MAX_COLLIDERS; ++j) {
            Collider* b = m_colliders[j];
            if (b == nullptr) continue;
            if (!canCollide(a, b)) continue;
            if (!intersects(a, b)) continue;

            const int p = pairIndex(i, j);
            setBit(m_currPairs, p);
            if (getBit(m_prevPairs, p)) dispatchStay(a, b);
            else                        dispatchEnter(a, b);
        }
    }

    // Second pass: fire Exit for pairs that were overlapping last frame but not this one.
    for (int i = 0; i < MAX_COLLIDERS; ++i) {
        Collider* a = m_colliders[i];
        if (a == nullptr) continue;

        for (int j = i + 1; j < MAX_COLLIDERS; ++j) {
            Collider* b = m_colliders[j];
            if (b == nullptr) continue;

            const int p = pairIndex(i, j);
            if (getBit(m_prevPairs, p) && !getBit(m_currPairs, p)) {
                dispatchExit(a, b);
            }
        }
    }

    m_prevPairs = m_currPairs;
}

bool ColliderManager::canCollide(const Collider* a, const Collider* b) {
    if (a == b) return false;
    if (a->getOwner() == b->getOwner()) return false;
    const uint16_t aToB = a->getMask() & static_cast<uint16_t>(b->getLayer());
    const uint16_t bToA = b->getMask() & static_cast<uint16_t>(a->getLayer());
    return (aToB != 0U) && (bToA != 0U);
}

bool ColliderManager::intersects(const Collider* a, const Collider* b) {
    using IntersectFn = bool (*)(const Collider*, const Collider*);
    static constexpr int SHAPE_COUNT = 2;
    const int ai = static_cast<int>(a->getShape());
    const int bi = static_cast<int>(b->getShape());
    if (ai < 0 || ai >= SHAPE_COUNT || bi < 0 || bi >= SHAPE_COUNT) return false;
    static const IntersectFn kIntersectTable[SHAPE_COUNT][SHAPE_COUNT] = {
        { &ColliderManager::intersectsBoxBox,    &ColliderManager::intersectsBoxCircle    },
        { &ColliderManager::intersectsBoxCircle, &ColliderManager::intersectsCircleCircle }
    };
    // Circle-Box case: swap args so the box is always first
    if (ai == 1 && bi == 0) return kIntersectTable[bi][ai](b, a);
    return kIntersectTable[ai][bi](a, b);
}

bool ColliderManager::intersectsBoxBox(const Collider* a, const Collider* b) {
    const float ax1 = a->getWorldX();
    const float ay1 = a->getWorldY();
    const float ax2 = ax1 + static_cast<float>(a->getWidth());
    const float ay2 = ay1 + static_cast<float>(a->getHeight());

    const float bx1 = b->getWorldX();
    const float by1 = b->getWorldY();
    const float bx2 = bx1 + static_cast<float>(b->getWidth());
    const float by2 = by1 + static_cast<float>(b->getHeight());

    return (ax1 < bx2) && (ax2 > bx1) && (ay1 < by2) && (ay2 > by1);
}

bool ColliderManager::intersectsCircleCircle(const Collider* a, const Collider* b) {
    const float dx = a->getWorldX() - b->getWorldX();
    const float dy = a->getWorldY() - b->getWorldY();
    const float r  = static_cast<float>(a->getRadius() + b->getRadius());
    return (dx * dx + dy * dy) <= (r * r);
}

bool ColliderManager::intersectsBoxCircle(const Collider* box, const Collider* circle) {
    const float bx = box->getWorldX();
    const float by = box->getWorldY();
    const float bw = static_cast<float>(box->getWidth());
    const float bh = static_cast<float>(box->getHeight());

    const float cx = circle->getWorldX();
    const float cy = circle->getWorldY();
    const float r  = static_cast<float>(circle->getRadius());

    const float closestX = std::clamp(cx, bx, bx + bw);
    const float closestY = std::clamp(cy, by, by + bh);
    const float dx = cx - closestX;
    const float dy = cy - closestY;
    return (dx * dx + dy * dy) <= (r * r);
}

void ColliderManager::clearPairsForSlot(int slot) {
    for (int i = 0; i < MAX_COLLIDERS; ++i) {
        if (i == slot) continue;
        const int p = pairIndex(slot, i);
        clearBit(m_prevPairs, p);
        clearBit(m_currPairs, p);
    }
}

// -------------------------------------
// Shape Independent Collision Event Dispatching
// -------------------------------------
void ColliderManager::dispatchEnter(Collider* a, Collider* b) const {
    if (a->isTrigger() || b->isTrigger()) {
        a->OnTriggerEnter(b);
        b->OnTriggerEnter(a);
    } else {
        a->OnCollisionEnter(b);
        b->OnCollisionEnter(a);
    }
}
void ColliderManager::dispatchStay(Collider* a, Collider* b) const {
    if (a->isTrigger() || b->isTrigger()) {
        a->OnTriggerStay(b);
        b->OnTriggerStay(a);
    } else {
        a->OnCollisionStay(b);
        b->OnCollisionStay(a);
    }
}
void ColliderManager::dispatchExit(Collider* a, Collider* b) const {
    if (a->isTrigger() || b->isTrigger()) {
        a->OnTriggerExit(b);
        b->OnTriggerExit(a);
    } else {
        a->OnCollisionExit(b);
        b->OnCollisionExit(a);
    }
}
// ----------------------------------------------