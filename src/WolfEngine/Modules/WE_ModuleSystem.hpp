#pragma once

class ModuleSystem {
private:
    friend class WolfEngine;
    static void InitAll();
    static void UpdateAll();
    static void ShutdownAll();
};
