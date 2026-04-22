#pragma once

class IModule {
public:
    const char* const Name;
    const int Priority;
    virtual ~IModule() = default;
private:
    friend class ModuleSystem;
    template<typename, int> friend class TModule;
    IModule(const char* name, int priority) : Name(name), Priority(priority) {}
    virtual void OnReferenceCollection() {}
    virtual void OnInit()                {}
    virtual void OnShutdown()            {}
    virtual void OnEarlyUpdate()         {}
    virtual void OnUpdate()              {}
    virtual void OnLateUpdate()          {}
    virtual void OnFreeUpdate()          {}
    virtual void OnPreRender()           {}
};

template<typename T, int MPriority> class TModule : public IModule {
public:
    static T& Get() { return *s_instance; }
protected:
    TModule(const char* name) : IModule(name, MPriority) { s_instance = static_cast<T*>(this); }
private:
    friend class ModuleSystem;
    inline static T* s_instance = nullptr;
};
