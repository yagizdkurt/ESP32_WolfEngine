#pragma once
#include <vector>
#include "WE_IModule.hpp"

class ModuleRegistry {
public:
    static void Register(IModule* mod)  { Modules().push_back(mod); }

    static void InitAll()               { for (auto* mod : Modules()) mod->OnInit(); }
    static void UpdateAll()             { for (auto* mod : Modules()) mod->OnUpdate(); }
    static void ShutdownAll() {
        auto& list = Modules();
        for (int i = list.size() - 1; i >= 0; i--)
            list[i]->OnShutdown();
    }

    template<typename T> static T* Get() {
        for (auto* mod : Modules())
            if (auto* typed = dynamic_cast<T*>(mod))
                return typed;
        return nullptr;
    }

private:
    static std::vector<IModule*>& Modules() {
        static std::vector<IModule*> s_modules;
        return s_modules;
    }
};

template<typename T> struct ModuleRegistrar { ModuleRegistrar() { ModuleRegistry::Register(new T()); } };