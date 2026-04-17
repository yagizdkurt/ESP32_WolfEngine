#pragma once
#include "stdint.h"
#include "stdbool.h"
#include <type_traits>
#include "WolfEngine/Utilities/WE_Vector2d.hpp"
#include "WolfEngine/ComponentSystem/Components/WE_Comp_Transform.hpp"
#include "WolfEngine/ComponentSystem/Components/WE_BaseComp.hpp"
class Collider;

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
    //  Update (virtual)
    //  Called every frame by the engine for all active objects.
    //  Override this in your subclass to implement game logic.
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
        obj->CreateObject();
        return obj;
    }


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


    // ---------------------------------------------------------
    //  registerComponent
    //  For internal component use only — do not call from game code.
    //  Components call this in their constructor to register
    //  themselves with their owning GameObject for automatic ticking.
    //  Calling this manually will result in undefined behavior.
    // ---------------------------------------------------------
    void registerComponent(Component* comp);

    // ---------------------------------------------------------
    // private members and functions below — not accessible from outside and not intended for users.
    // ----------------------------------------------------------
protected:
    GameObject();
    virtual ~GameObject();
private:
    static constexpr int MAX_COMPONENTS_PER_OBJECT = COMP_MAX_TYPE;
    Component* m_components[MAX_COMPONENTS_PER_OBJECT] = {};
    int        m_componentCount = 0;
    uint8_t  id = -1;
    bool     isValid = false;
    bool     hasStarted = false;
    void componentTick();
    bool CreateObject();
    void callStart();
    friend class WolfEngine;

};