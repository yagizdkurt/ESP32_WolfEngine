#pragma once
// SDL input bridge — mirrors the pattern of Display_SDL.h.
// Forward-declares the flush function implemented in WE_SDLInputDriver.cpp.
// Included by WE_InputManager.cpp under #ifdef WE_PLATFORM_SDL.
class Controller;
void SDLInput_flush(Controller& c);
