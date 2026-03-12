#include "LED.h"

void LED::begin() {
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(_leds, NUM_LEDS);
    FastLED.setBrightness(50);
    _state         = false;
    _lastToggle    = 0;
    _blinkCount    = 0;
    _blinkInterval = 0;
    _continuous    = false;
    _blinkColor    = CRGB::White;
    off();
}

void LED::setColor(CRGB color) {
    _continuous = false;
    _blinkCount = 0;
    _leds[0]    = color;
    FastLED.show();
}

void LED::off() {
    _continuous = false;
    _blinkCount = 0;
    _leds[0]    = CRGB::Black;
    FastLED.show();
}

void LED::blink(CRGB color, uint8_t times, uint16_t interval_ms) {
    _continuous    = false;
    _blinkColor    = color;
    _blinkCount    = times * 2;
    _blinkInterval = interval_ms;
    _lastToggle    = millis();
    _state         = true;
    _leds[0]       = color;
    FastLED.show();
}

void LED::fastBlink(CRGB color) {
    _continuous    = true;
    _blinkColor    = color;
    _blinkInterval = 100;
    _lastToggle    = millis();
}

void LED::slowBlink(CRGB color) {
    _continuous    = true;
    _blinkColor    = color;
    _blinkInterval = 500;
    _lastToggle    = millis();
}

void LED::setIdle()    { setColor(CRGB::Green);      }
void LED::setOpening() { slowBlink(CRGB::Green);     }
void LED::setClosing() { slowBlink(CRGB::Red);       }
void LED::setInit()    { fastBlink(CRGB::Yellow);    }
void LED::setSaved()   { blink(CRGB::Blue, 3, 200); }
void LED::setError()   { fastBlink(CRGB::Red);       }
void LED::setUnknown() { setColor(CRGB::Orange); }  // Turuncu sabit

void LED::update() {
    if (_continuous) {
        if (millis() - _lastToggle >= _blinkInterval) {
            _state   = !_state;
            _leds[0] = _state ? _blinkColor : CRGB::Black;
            FastLED.show();
            _lastToggle = millis();
        }
        return;
    }

    if (_blinkCount > 0) {
        if (millis() - _lastToggle >= _blinkInterval) {
            _state   = !_state;
            _leds[0] = _state ? _blinkColor : CRGB::Black;
            FastLED.show();
            _lastToggle = millis();
            _blinkCount--;
        }
    }
}