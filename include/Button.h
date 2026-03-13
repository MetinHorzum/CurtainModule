#pragma once
#include <Arduino.h>

class Button {
public:
    Button(uint8_t pin);

    void begin();
    void update();

    bool isShortPress();            // < 1sn
    bool isLongPress(uint16_t ms);  // ms kadar basili tutuldu mu?

private:
    uint8_t  _pin;
    bool     _lastState;
    bool     _shortPress;
    uint32_t _pressTime;
    bool     _longFired;

    static const uint16_t DEBOUNCE_MS = 50;
};