#pragma once
#include <Arduino.h>

class Encoder {
public:
    Encoder(uint8_t pin);
    void     begin();
    void     update();              // ISR'dan cagrilir
    uint32_t getCount();
    uint32_t getLastPulseTime();
    void     resetCount();
    bool     isStalled(uint16_t timeout_ms);

private:
    uint8_t           _pin;
    volatile uint32_t _count;
    volatile uint32_t _lastPulseTime;
};