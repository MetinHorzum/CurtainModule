#include "Button.h"

Button::Button(uint8_t pin) {
    _pin        = pin;
    _lastState  = HIGH;
    _shortPress = false;
    _pressTime  = 0;
    _longFired  = false;
    _firedAt    = 0;
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
        _firedAt   = 0;
    }

    // Bırakıldı
    if (_lastState == LOW && current == HIGH) {
        if (!_longFired && (millis() - _pressTime >= DEBOUNCE_MS)) {
            _shortPress = true;
        }
        _pressTime = 0;
        _longFired = false;
        _firedAt   = 0;
    }

    _lastState = current;
}

bool Button::isShortPress() {
    return _shortPress;
}

bool Button::isLongPress(uint16_t ms) {
    // Buton basılı mı?
    if (digitalRead(_pin) != LOW) return false;
    // Daha önce bu süre için tetiklendi mi?
    if (_longFired && _firedAt == ms) return false;
    // Süre doldu mu?
    if (_pressTime > 0 && (millis() - _pressTime >= ms)) {
        if (!_longFired || _firedAt != ms) {
            _longFired = true;
            _firedAt   = ms;
            return true;
        }
    }
    return false;
}