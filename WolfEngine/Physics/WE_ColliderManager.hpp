#include "WolfEngine/Settings/WE_Settings.hpp"
class Collider;


class ColliderManager {
public:
    void registerCollider(Collider* collider);
    void unregisterCollider(Collider* collider);
private:
    Collider* m_colliders[MAX_GAME_OBJECTS * 3] = { };
    friend class Collider;
};