#pragma once
#include <Arduino.h>

class Button {
public:
    Button(uint8_t pin);

    void begin();
    void update();

    bool isShortPress();
    bool isLongPress(uint16_t ms);

private:
    uint8_t  _pin;
    bool     _lastState;
    bool     _shortPress;
    uint32_t _pressTime;
    uint16_t _firedAt;     // Kaç ms'de tetiklendi?
    bool     _longFired;

    static const uint16_t DEBOUNCE_MS = 50;
};