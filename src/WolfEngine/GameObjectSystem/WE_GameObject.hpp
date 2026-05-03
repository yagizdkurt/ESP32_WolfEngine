#pragma once
#include "stdint.h"
#include "stdbool.h"
#include "WolfEngine/Settings/WE_Settings.hpp"
#include "WolfEngine/Utilities/Debug/WE_Debug.hpp"
#include <type_traits>
#include "WolfEngine/Utilities/WE_Vector2d.hpp"
#include "WolfEngine/ComponentSystem/Components/WE_Comp_Transform.hpp"
#include "WolfEngine/ComponentSystem/Components/WE_BaseComp.hpp"

#if defined(WE_MODULE_COLLISION)
class Collider;
#endif

// =============================================================
//                  GAMEOBJECT SYSTEM
// =============================================================
//  All game entities inherit from GameObject. It provides a
//  transform, an update/start lifecycle, and a factory for
//  safe creation through the engine registry.
//
//  CREATING OBJECTS
//  ----------------
//  Always use GameObject::Create<T>() to instantiate objects.
//  Never use new directly — doing so bypasses the registry and
//  the engine will not manage the object's lifetime.
//
//      Good:
//          Player* p = GameObject::Create<Player>();
//
//      Bad:
//          Player* p = new Player(); // not registered, not managed
//
//  DESTROYING OBJECTS
//  ------------------
//  Always destroy objects through the provided destroy functions.
//  Never call delete directly on a GameObject pointer.
//
//      Good:
//          player->destroyGameObject();
//
//      Bad:
//          delete player; // bypasses registry cleanup
// =============================================================

class GameObject {
public:

    // ---------------------------------------------------------
    //  isActive
    //  Controls whether this object receives Update() calls
    //  each frame. Set to false to temporarily pause an object
    //  without removing it from the registry.
    //
    //      Good:
    //          enemy->isActive = false; // pause enemy during cutscene
    //          enemy->isActive = true;  // resume after cutscene
    //
    //      Bad:
    //          GameObject::DestroyGameObject(enemy); // destroying when you only want to pause
    // ---------------------------------------------------------
    bool isActive = true;

    uint16_t m_updateLayer = UL_UPDATE_GAMEPLAY;
    void setUpdateLayer(uint16_t layer) { m_updateLayer = layer; }

    // ---------------------------------------------------------
    //  transform
    //  Every GameObject has a transform component by default.
    //  Use it to get and set the object's position in the world.
    //  Never create a separate transform — use this one.
    //
    //      Good:
    //          player->transform.position = {64, 64};
    //
    // ---------------------------------------------------------
    TransformComponent transform;


    // ---------------------------------------------------------
    //  DestroyGameObject (static)
    //  Destroys the given object, removes it from the registry,
    //  and frees its memory. Use this when you have a pointer
    //  to an object from outside of it.
    //
    //      Good:
    //          GameObject::DestroyGameObject(enemy);
    //
    //      Bad:
    //          delete enemy; // registry still holds a dangling pointer
    // ---------------------------------------------------------
    static void DestroyGameObject(GameObject* gameObject);


    // ---------------------------------------------------------
    //  destroyGameObject
    //  Destroys this object from within itself. Convenient to
    //  call from inside Update() when the object decides it
    //  should no longer exist (e.g. bullet hits a wall).
    //
    //      Good:
    //          void Update() override {
    //              if (hitWall) destroyGameObject();
    //          }
    //
    //      Bad:
    //          void Update() override {
    //              if (hitWall) delete this; // dangerous, bypasses registry
    //          }
    // ---------------------------------------------------------
    void destroyGameObject() { DestroyGameObject(this); }

    // ---------------------------------------------------------
    //  EarlyUpdate (virtual)
    //  Called every frame in the Early phase from
    //  WolfEngine::gameTick() for all active objects.
    //  Order in Early phase:
    //      1) Input tick
    //      2) GameObject::EarlyUpdate()
    //      3) earlyComponentTick()
    //      4) ModuleSystem::EarlyUpdate()
    //  Use this for input-driven prep before main game logic.
    //
    //      Good:
    //          void EarlyUpdate() override {
    //              // cache input state for this frame
    //          }
    //
    //      Bad:
    //          player->EarlyUpdate(); // manual call breaks frame ordering
    // ---------------------------------------------------------
    virtual void EarlyUpdate() {}

    // ---------------------------------------------------------
    //  Update (virtual)
    //  Called every frame in the Main phase from
    //  WolfEngine::gameTick() for all active objects.
    //  Order in Main phase:
    //      1) GameObject::Update()
    //      2) componentTick()
    //      3) ModuleSystem::Update()
    //  Override this in your subclass for main game logic.
    //  Do not call this manually — the engine drives it.
    //
    //      Good:
    //          void Update() override {
    //              transform.position.x += speed;
    //          }
    //
    //      Bad:
    //          player->Update(); // calling manually breaks frame ordering
    // ---------------------------------------------------------
    virtual void Update() {}

    // ---------------------------------------------------------
    //  LateUpdate (virtual)
    //  Called every frame in the Late phase from
    //  WolfEngine::gameTick() for all active objects.
    //  Order in Late phase:
    //      1) GameObject::LateUpdate()
    //      2) lateComponentTick()
    //      3) ModuleSystem::LateUpdate()
    //      4) Camera follow tick
    //      5) ColliderManager tick (temporary legacy call)
    //  Use this for post-logic cleanup/finalization.
    //
    //      Good:
    //          void LateUpdate() override {
    //              // finalize values after main logic
    //          }
    //
    //      Bad:
    //          enemy->LateUpdate(); // manual call breaks frame ordering
    // ---------------------------------------------------------
    virtual void LateUpdate() {}

    // ---------------------------------------------------------
    //  start (virtual)
    //  Called once by the engine immediately after the object
    //  is created via Create<T>(). Use this for initialization
    //  that depends on other systems being ready, rather than
    //  putting logic in a constructor.
    //
    //      Good:
    //          void start() override {
    //              transform.position = {64, 64};
    //          }
    //
    //      Bad:
    //          Player() { transform.position = {64, 64}; } // constructor runs before engine setup
    // ---------------------------------------------------------
    virtual void Start() {}

    //  ---------------------------------------------------------
    //  Getters for object state
    //  isUpdatable / IsDead / IsValid / IsActive
    bool isUpdatable() { return !isDead && isActive; }
    bool IsDead() const { return isDead; }
    bool IsValid() const { return isValid; }
    bool IsActive() const { return isActive; }

    // ---------------------------------------------------------
    //  Create<T> (static factory)
    //  The only correct way to instantiate a GameObject or any
    //  subclass. Allocates the object, registers it with the
    //  engine, and calls start(). Returns nullptr if the registry
    //  is full or creation fails — always check the return value.
    //
    //  T must be a subclass of GameObject.
    //
    //      Good:
    //          Player* p = GameObject::Create<Player>();
    //          if (!p) { // handle registry full }
    //
    //      Bad:
    //          Player* p = new Player(); // not registered, start() not called
    // ---------------------------------------------------------
    template<typename T> static T* Create() {
        static_assert(std::is_base_of<GameObject, T>::value, "T must be a GameObject");
        T* obj = new T();
        if (!obj->CreateObject()) {
            delete obj;
            WE_PANIC("Create<T>() failed: registry full or CreateObject() error. "
                     "You can expand the registry in settings, but given that ESP32 has limited RAM "
                     "we recommend you check your design choices.");
        }
        return obj;
    }

#if defined(WE_MODULE_COLLISION)
    // ---------------------------------------------------------
    //  Collision and Trigger Events (virtual)
    //  Override these in your subclass to respond to collisions
    //  and triggers. These are called by the engine when the
    //  appropriate physics events occur. Do not call these manually.
    // ---------------------------------------------------------
    virtual void OnCollisionEnter (Collider* other)  { }
    virtual void OnCollisionStay  (Collider* other)  { }
    virtual void OnCollisionExit  (Collider* other)  { }
    virtual void OnTriggerEnter   (Collider* other)  { }
    virtual void OnTriggerStay    (Collider* other)  { }
    virtual void OnTriggerExit    (Collider* other)  { }
#endif

    // ---------------------------------------------------------
    //  registerComponent
    //  For internal component use only — do not call from game code.
    //  Components call this in their constructor to register
    //  themselves with their owning GameObject for automatic ticking.
    //  Calling this manually will result in undefined behavior.
    // ---------------------------------------------------------
    void registerComponent(Component* comp);

protected:
    GameObject();
    virtual ~GameObject();
private:
    static constexpr int MAX_COMPONENTS_PER_OBJECT = COMP_MAX_TYPE;
    Component* m_components[MAX_COMPONENTS_PER_OBJECT] = {};
    int        m_componentCount = 0;
    static constexpr uint16_t INVALID_ID = 0xFFFF;
    uint16_t id = INVALID_ID;

    bool     hasStarted = false;

    bool     isValid = false;
    bool     isDead = false;

    void earlyComponentTick();
    void componentTick();
    void lateComponentTick();
    void preRenderComponentTick();
    bool CreateObject();
    void callStart();

    friend class WolfEngine;

};
