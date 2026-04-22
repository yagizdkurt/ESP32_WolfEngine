#pragma once
#include "WolfEngine/Settings/WE_Settings.hpp"
#if defined(WE_MODULE_COLLISION)

#include <stdint.h>
#include "WolfEngine/Modules/WE_IModule.hpp"

class Collider;

class WE_CollisionModule : public TModule<WE_CollisionModule, 0> {
public:
    WE_CollisionModule();
    void RegisterCollider  (Collider* collider);
    void UnregisterCollider(Collider* collider);

private:
    static constexpr int MAX_COLLIDERS = Settings.limits.maxGameObjects * 3;
    static constexpr int PAIR_BITS     = (MAX_COLLIDERS * (MAX_COLLIDERS - 1)) / 2;
    static constexpr int PAIR_BYTES    = (PAIR_BITS + 7) / 8;

    Collider* m_colliders[MAX_COLLIDERS] = {};
    uint8_t   m_prevPairs[PAIR_BYTES]    = {};
    uint8_t   m_currPairs[PAIR_BYTES]    = {};
    int       m_freeStack[MAX_COLLIDERS] = {};
    int       m_freeTop = 0;

    void OnLateUpdate() override;

    static bool intersects            (const Collider* a, const Collider* b);
    static bool intersectsBoxBox      (const Collider* a, const Collider* b);
    static bool intersectsCircleCircle(const Collider* a, const Collider* b);
    static bool intersectsBoxCircle   (const Collider* box, const Collider* circle);

    static bool canCollide(const Collider* a, const Collider* b);
    void dispatchEnter(Collider* a, Collider* b) const;
    void dispatchStay (Collider* a, Collider* b) const;
    void dispatchExit (Collider* a, Collider* b) const;
    void clearPairsForSlot(int slot);

    static constexpr bool getBit (const uint8_t* b, int i) noexcept { return (b[i >> 3] & static_cast<uint8_t>(1U << (i & 7))) != 0U; }
    static constexpr void setBit (      uint8_t* b, int i) noexcept { b[i >> 3] |=  static_cast<uint8_t>(1U << (i & 7)); }
    static constexpr void clearBit(     uint8_t* b, int i) noexcept { b[i >> 3] &= static_cast<uint8_t>(~(1U << (i & 7))); }

    static constexpr int pairIndex(int a, int b) noexcept
        { if (a > b) { int t = a; a = b; b = t; } return (a * (2 * MAX_COLLIDERS - a - 1)) / 2 + (b - a - 1); }

    static constexpr float clamp(float v, float lo, float hi) noexcept
        { return v < lo ? lo : v > hi ? hi : v; }

    friend class Collider;
};

#endif // WE_MODULE_COLLISION
