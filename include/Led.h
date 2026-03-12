#pragma once
#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS       1
#define LED_TYPE       WS2812
#define COLOR_ORDER    GRB
#define LED_DATA_PIN   5

class LED {
public:
    void begin();
    void update();

    void setColor(CRGB color);
    void off();

    void blink(CRGB color, uint8_t times, uint16_t interval_ms);
    void fastBlink(CRGB color);
    void slowBlink(CRGB color);

    void setIdle();
    void setOpening();
    void setClosing();
    void setInit();
    void setSaved();
    void setError();
    void setUnknown();    // EEPROM yok → Turuncu sabit

private:
    CRGB     _leds[NUM_LEDS];
    bool     _state;
    uint32_t _lastToggle;
    int16_t  _blinkCount;
    uint16_t _blinkInterval;
    bool     _continuous;
    CRGB     _blinkColor;
};