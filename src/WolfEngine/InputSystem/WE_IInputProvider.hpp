#pragma once

class Controller;

// Abstract input provider interface.
// Implement this on any platform that wants to override hardware input polling.
// Set via InputManager::setInputProvider() before the game loop starts.
// nullptr = use hardware GPIO/ADC path (default).
class IInputProvider {
public:
    virtual void flush(Controller* controllers, int count) = 0;
    virtual ~IInputProvider() = default;
};
