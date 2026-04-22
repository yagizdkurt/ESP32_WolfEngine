#pragma once

class ModuleSystem {
private:
    friend class WolfEngine;

    static void InitAll();
    static void ShutdownAll();
    
    static void EarlyUpdate();
    static void Update();
    static void LateUpdate();
    static void PreRender();
    static void FreeUpdate();
};
