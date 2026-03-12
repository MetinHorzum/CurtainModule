#pragma once
#include <Arduino.h>

class Button {
public:
    Button(uint8_t pin);

    void begin();
    void update();

    bool isShortPress();
    bool isLongPress();
    bool isDoublePress();   // YENİ

private:
    uint8_t  _pin;
    bool     _lastState;
    bool     _shortPress;
    bool     _longPress;
    bool     _doublePress;
    uint32_t _pressTime;
    uint32_t _lastReleaseTime;
    bool     _longFired;
    uint8_t  _pressCount;

    static const uint16_t SHORT_PRESS_MS  = 50;
    static const uint16_t LONG_PRESS_MS   = 3000;
    static const uint16_t DOUBLE_PRESS_MS = 400;
};