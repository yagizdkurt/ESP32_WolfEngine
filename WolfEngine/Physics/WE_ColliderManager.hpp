#pragma once
#include "WolfEngine/Settings/WE_Settings.hpp"
#include <array>
#include <cstdint>

class Collider;

class ColliderManager {
public:
    void registerCollider(Collider* collider);
    void unregisterCollider(Collider* collider);

private:
    static constexpr int MAX_COLLIDERS = MAX_GAME_OBJECTS * 3;
    static constexpr int PAIR_BITS     = (MAX_COLLIDERS * (MAX_COLLIDERS - 1)) / 2;
    static constexpr int PAIR_BYTES    = (PAIR_BITS + 7) / 8;

    using ColliderArray = std::array<Collider*, MAX_COLLIDERS>;
    using PairBits      = std::array<uint8_t,   PAIR_BYTES>;
    using FreeStack     = std::array<int,       MAX_COLLIDERS>;

    ColliderArray m_colliders = { };
    PairBits      m_prevPairs = { };
    PairBits      m_currPairs = { };
    FreeStack     m_freeStack = { };   // stack of free slot indices
    int           m_freeTop   = 0;     // m_freeTop == 0 means no free slots

    // ---------------- shape specific -----------------
    static bool intersects(const Collider* a, const Collider* b);
    static bool intersectsBoxBox(const Collider* a, const Collider* b);
    static bool intersectsCircleCircle(const Collider* a, const Collider* b);
    static bool intersectsBoxCircle(const Collider* box, const Collider* circle);

    // ---------------- shape independent -----------------
    static bool canCollide(const Collider* a, const Collider* b);
    void dispatchEnter(Collider* a, Collider* b) const;
    void dispatchStay (Collider* a, Collider* b) const;
    void dispatchExit (Collider* a, Collider* b) const;
    void clearPairsForSlot(int slot);

    // Bit manipulation helpers for the pair bit arrays.
    static constexpr bool getBit(const PairBits& bits, int idx) noexcept { return (bits[static_cast<size_t>(idx >> 3)] & static_cast<uint8_t>(1U << (idx & 7))) != 0U; }
    static constexpr void setBit(PairBits& bits, int idx) noexcept { bits[static_cast<size_t>(idx >> 3)] |= static_cast<uint8_t>(1U << (idx & 7)); }
    static constexpr void clearBit(PairBits& bits, int idx) noexcept { bits[static_cast<size_t>(idx >> 3)] &= static_cast<uint8_t>(~(1U << (idx & 7))); }
    static void clearBits(PairBits& bits) noexcept { bits.fill(0U); }

    // Maps an ordered pair (a < b) into the triangular-packed bit array. Enforces ordering so pairIndex(a,b) == pairIndex(b,a).
    static constexpr int pairIndex(int a, int b) noexcept { if (a > b) { int t = a; a = b; b = t; } return (a * (2 * MAX_COLLIDERS - a - 1)) / 2 + (b - a - 1); }

    // ---- Engine Handled -----
    friend class Collider;
    friend class WolfEngine;
    ColliderManager() noexcept;
    void tick();
};
