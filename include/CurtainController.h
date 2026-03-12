#pragma once
#include <Arduino.h>
#include "Motor.h"
#include "Encoder.h"
#include "LED.h"
#include "Button.h"
#include "Settings.h"
#include "config.h"

// ─── State Machine ────────────────────────────────────
enum CurtainState {
    STATE_UNKNOWN,      // EEPROM yok, pozisyon bilinmiyor
    STATE_IDLE,         // Durdu
    STATE_OPENING,      // Aciliyor
    STATE_CLOSING,      // Kapaniyor
    STATE_INIT          // Init modu
};

class CurtainController {
public:
    CurtainController(Motor& motor, Encoder& encoder,
                      LED& led, Button& button);

    void begin();
    void update();          // Loop'ta cagrilmali
    void runInit();         // Init sekansini calistir

    uint8_t getPosition();  // 0-100%
    CurtainState getState();

private:
    Motor&   _motor;
    Encoder& _encoder;
    LED&     _led;
    Button&  _button;

    CurtainState  _state;
    CurtainSettings _settings;
    uint8_t       _position;    // 0-100%
    bool          _initialized;

    // Hareket
    void _moveTo(uint8_t targetPos);
    void _stop();
    void _updatePosition();
    void _updateLED();

    // Perde moduna gore hedef hesapla
    uint8_t _getOpenTarget();
    uint8_t _getCloseTarget();
};