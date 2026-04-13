// Phase 2 desktop display driver.
// GetSDLDriver() owns the single static SDL3DisplayDriver instance.
// GetDriver()    satisfies the WolfEngine constructor (returns the same object as DisplayDriver*).
// main_desktop.cpp calls GetSDLDriver().setRenderer() before StartEngine()
// and GetSDLDriver().destroy() after engineThread.join().

#include "WE_Display_SDL3.hpp"

SDL3DisplayDriver& GetSDLDriver() {
    static SDL3DisplayDriver driver;
    return driver;
}

DisplayDriver* GetDriver() { return &GetSDLDriver(); }