#pragma once
#include <vector>
#include "WE_IModule.hpp"

class ModuleRegistry {
public:
    // Insert in descending priority order (highest priority runs first).
    static void Register(IModule* mod) { 
        auto& list = Modules();
        auto it = list.begin();
        while (it != list.end() && (*it)->GetPriority() >= mod->GetPriority()) ++it;
        list.insert(it, mod);
    }

    static void InitAll()               { for (auto* mod : Modules()) mod->OnInit(); }
    static void UpdateAll()             { for (auto* mod : Modules()) mod->OnUpdate(); }
    static void ShutdownAll() {
        auto& list = Modules();
        for (int i = list.size() - 1; i >= 0; i--) list[i]->OnShutdown();
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

template<typename T> struct ModuleRegistrar {
    ModuleRegistrar() {
        static T s_instance;
        ModuleRegistry::Register(&s_instance);
    }
};
