#pragma once
class IModule {
public:
    virtual ~IModule() = default;
    virtual const char* GetName()     const = 0;
    virtual int         GetPriority() const = 0;

    virtual void OnInit()     {}
    virtual void OnUpdate()   {}
    virtual void OnShutdown() {}
};