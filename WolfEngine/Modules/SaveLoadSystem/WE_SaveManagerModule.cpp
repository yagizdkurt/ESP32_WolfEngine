#include "WolfEngine/Settings/WE_Modules.hpp"

#ifdef SaveLoadModule
#include "WE_SaveManager.hpp"
#include "WolfEngine/Modules/WE_ModuleRegistry.hpp"
static ModuleRegistrar<WE_SaveManager> s_registrar;
#endif