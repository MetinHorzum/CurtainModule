#include "Button.h"

Button::Button(uint8_t pin) {
    _pin       = pin;
    _lastState = HIGH;
    _shortPress = false;
    _pressTime  = 0;
    _longFired  = false;
}

void Button::begin() {
    pinMode(_pin, INPUT_PULLUP);
}

void Button::update() {
    bool current = digitalRead(_pin);
    _shortPress  = false;

    // Basıldı
    if (_lastState == HIGH && current == LOW) {
        _pressTime = millis();
        _longFired = false;
    }

    // Bırakıldı
    if (_lastState == LOW && current == HIGH) {
        if (!_longFired && (millis() - _pressTime >= DEBOUNCE_MS)) {
            _shortPress = true;
        }
    }

    _lastState = current;
}

bool Button::isShortPress() {
    return _shortPress;
}

bool Button::isLongPress(uint16_t ms) {
    if (digitalRead(_pin) == LOW && !_longFired) {
        if (_pressTime > 0 && (millis() - _pressTime >= ms)) {
            _longFired = true;
            return true;
        }
    }
    return false;
}