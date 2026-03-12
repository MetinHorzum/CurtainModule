#include "Button.h"

Button::Button(uint8_t pin) {
    _pin             = pin;
    _lastState       = HIGH;
    _shortPress      = false;
    _longPress       = false;
    _doublePress     = false;
    _pressTime       = 0;
    _lastReleaseTime = 0;
    _longFired       = false;
    _pressCount      = 0;
}

void Button::begin() {
    pinMode(_pin, INPUT_PULLUP);
}

void Button::update() {
    bool current = digitalRead(_pin);
    _shortPress  = false;
    _longPress   = false;
    _doublePress = false;

    // Butona basıldı
    if (_lastState == HIGH && current == LOW) {
        _pressTime = millis();
        _longFired = false;
        _pressCount++;
    }

    // Basılı tutuluyor
    if (current == LOW && !_longFired) {
        if (millis() - _pressTime >= LONG_PRESS_MS) {
            _longPress = true;
            _longFired = true;
            _pressCount = 0;
        }
    }

    // Bırakıldı
    if (_lastState == LOW && current == HIGH) {
        _lastReleaseTime = millis();
        if (!_longFired) {
            if (millis() - _pressTime >= SHORT_PRESS_MS) {
                // Çift bas kontrolü
                if (_pressCount >= 2) {
                    _doublePress = true;
                    _pressCount  = 0;
                }
            }
        }
    }

    // Double press zaman aşımı
    if (_pressCount == 1 &&
        millis() - _lastReleaseTime > DOUBLE_PRESS_MS &&
        current == HIGH) {
        _shortPress = true;
        _pressCount = 0;
    }

    _lastState = current;
}

bool Button::isShortPress()  { return _shortPress;  }
bool Button::isLongPress()   { return _longPress;   }
bool Button::isDoublePress() { return _doublePress; }