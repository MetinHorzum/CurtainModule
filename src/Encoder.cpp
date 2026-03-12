#include "Encoder.h"

Encoder::Encoder(uint8_t pin) {
    _pin           = pin;
    _count         = 0;
    _lastPulseTime = 0;
}

void Encoder::begin() {
    pinMode(_pin, INPUT_PULLUP);
}

void Encoder::update() {
    _count++;
    _lastPulseTime = millis();
}

uint32_t Encoder::getCount() {
    noInterrupts();
    uint32_t c = _count;
    interrupts();
    return c;
}

uint32_t Encoder::getLastPulseTime() {
    noInterrupts();
    uint32_t t = _lastPulseTime;
    interrupts();
    return t;
}

void Encoder::resetCount() {
    noInterrupts();
    _count         = 0;
    _lastPulseTime = millis();
    interrupts();
}

bool Encoder::isStalled(uint16_t timeout_ms) {
    if (getLastPulseTime() == 0) return false;
    return (millis() - getLastPulseTime()) > timeout_ms;
}