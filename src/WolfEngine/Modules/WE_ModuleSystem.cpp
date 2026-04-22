#include "WE_ModuleSystem.hpp"
#include "WE_IModule.hpp"
#include "WolfEngine/Settings/WE_Settings.hpp"

// ── Module instances ──────────────────────────────────────────────────────────
#if defined(WE_MODULE_SAVELOAD)
#include "SaveLoadSystem/WE_SaveManager.hpp"
static WE_SaveManager s_saveLoad;
#endif

// ── Module list ───────────────────────────────────────────────────────────────
static IModule* s_modules[] = {
#if defined(WE_MODULE_SAVELOAD)
    &s_saveLoad,
#endif
};




// ── ModuleSystem implementation ─────────────────────────────────────────────
// Nothing to change here...
// ── Module count ─────────────────────────────────────────────────────────────
static constexpr int s_count = sizeof(s_modules) / sizeof(IModule*);
// ── Module lifecycle ──────────────────────────────────────────────────────────
void ModuleSystem::InitAll() { // Sort by priority first, then get references, then init in that order
    static bool sorted = false;
    if (!sorted) { //Sorting is in descending order (highest priority first)
        for (int i = 1; i < s_count; i++) {
            IModule* key = s_modules[i];
            int j = i - 1;
            while (j >= 0 && s_modules[j]->Priority < key->Priority) {
                s_modules[j + 1] = s_modules[j];
                j--;
            }
            s_modules[j + 1] = key;
        }
        sorted = true;
    }
    for (int i = 0; i < s_count; i++) s_modules[i]->OnReferenceCollection();
    for (int i = 0; i < s_count; i++) s_modules[i]->OnInit();
}

void ModuleSystem::EarlyUpdate() { for (int i = 0; i < s_count; i++)      s_modules[i]->OnEarlyUpdate(); }
void ModuleSystem::Update()      { for (int i = 0; i < s_count; i++)      s_modules[i]->OnUpdate();      }
void ModuleSystem::LateUpdate()  { for (int i = 0; i < s_count; i++)      s_modules[i]->OnLateUpdate();  }
void ModuleSystem::PreRender()   { for (int i = 0; i < s_count; i++)      s_modules[i]->OnPreRender();   }
void ModuleSystem::FreeUpdate()  { for (int i = 0; i < s_count; i++)      s_modules[i]->OnFreeUpdate();  }
void ModuleSystem::ShutdownAll() { for (int i = s_count - 1; i >= 0; i--) s_modules[i]->OnShutdown();    }