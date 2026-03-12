#include "CurtainController.h"

CurtainController::CurtainController(Motor& motor, Encoder& encoder,
                                     LED& led, Button& button)
    : _motor(motor), _encoder(encoder), _led(led), _button(button)
{
    _state       = STATE_UNKNOWN;
    _position    = 0;
    _initialized = false;
}

// ─── Begin ────────────────────────────────────────────
void CurtainController::begin() {
    if (Settings::load(_settings)) {
        _initialized = true;
        _position    = _settings.lastPosition;
        _state       = STATE_IDLE;
        Serial.print("Ayarlar yuklendi! Pulse: ");
        Serial.println(_settings.totalPulses);
        Serial.print("Son pozisyon: %");
        Serial.println(_position);
    } else {
        _initialized = false;
        _state       = STATE_UNKNOWN;
        Serial.println("EEPROM bos! Init gerekli.");
    }
    _updateLED();
}

// ─── Update ───────────────────────────────────────────
void CurtainController::update() {
    _button.update();
    _led.update();

    // Init modu
    if (_state == STATE_UNKNOWN) {
        // 3sn uzun bas → init baslat
        if (_button.isLongPress()) {
            runInit();
        }
        return;
    }

    // Normal mod buton kontrolleri
    if (_button.isDoublePress()) {
        // Çift bas → Acil dur
        _stop();
        Serial.println("Acil dur!");
        return;
    }

    if (_button.isLongPress()) {
        // Uzun bas → Init sifirla
        Settings::reset();
        _initialized = false;
        _state       = STATE_UNKNOWN;
        Serial.println("Ayarlar silindi!");
        _updateLED();
        return;
    }

    if (_button.isShortPress()) {
        if (_state == STATE_OPENING || _state == STATE_CLOSING) {
            // Hareket ediyorsa → Dur
            _stop();
        } else {
            // Duruyorsa → Toggle
            uint8_t target = (_position < 50)
                             ? _getOpenTarget()
                             : _getCloseTarget();
            _moveTo(target);
        }
        return;
    }

    // Hareket halindeyse pozisyon guncelle
    if (_state == STATE_OPENING || _state == STATE_CLOSING) {
        _updatePosition();

        // Stall kontrolu
        if (_encoder.isStalled(STALL_TIMEOUT_MS)) {
            _stop();
            Serial.println("Stall - Hedefe ulasti!");
        }

        // NFAULT kontrolu
        if (_motor.isFault()) {
            _stop();
            _state = STATE_UNKNOWN;
            _led.setError();
            Serial.println("Motor HATA!");
        }
    }
}

// ─── Run Init ─────────────────────────────────────────
void CurtainController::runInit() {
    _state = STATE_INIT;
    Serial.println("=== INIT MODU ===");
    _led.setInit();

    Serial.println("Butona bas -> Baslat");
    while (!_button.isShortPress()) {
        _button.update();
        _led.update();
    }

    // Adim 1: Kapat → stall = 0%
    Serial.println("Kapanıyor...");
    _led.setClosing();
    _encoder.resetCount();
    _motor.forward(MOTOR_PWM_INIT);

    while (true) {
        _led.update();
        if (_encoder.isStalled(STALL_TIMEOUT_MS)) {
            _motor.brake();
            Serial.println("0% bulundu!");
            break;
        }
    }

    delay(2000);
    _encoder.resetCount();

    // Adim 2: Ac → stall = 100%
    Serial.println("Aciliyor...");
    _led.setOpening();
    _motor.backward(MOTOR_PWM_INIT);

    while (true) {
        _led.update();
        if (_encoder.isStalled(STALL_TIMEOUT_MS)) {
            _motor.brake();
            Serial.print("100% bulundu! Pulse: ");
            Serial.println(_encoder.getCount());
            break;
        }
    }

    // Kaydet
    _settings.totalPulses  = _encoder.getCount();
    _settings.lastPosition = 100;
    _settings.isInitialized = true;
    Settings::save(_settings);

    _initialized = true;
    _position    = 100;
    _state       = STATE_IDLE;

    Serial.println("Kaydedildi!");
    _led.setSaved();
    delay(1500);
    _updateLED();
}

// ─── Move To ──────────────────────────────────────────
void CurtainController::_moveTo(uint8_t targetPos) {
    if (!_initialized) return;
    if (targetPos == _position) return;

    Serial.print("Hedef: %");
    Serial.println(targetPos);

    if (targetPos > _position) {
        _state = STATE_OPENING;
        _motor.backward(MOTOR_PWM_NORMAL);
        _led.setOpening();
    } else {
        _state = STATE_CLOSING;
        _motor.forward(MOTOR_PWM_NORMAL);
        _led.setClosing();
    }

    _encoder.resetCount();
}

// ─── Stop ─────────────────────────────────────────────
void CurtainController::_stop() {
    _motor.brake();
    _updatePosition();
    _state = STATE_IDLE;

    // Pozisyonu EEPROM'a kaydet
    _settings.lastPosition = _position;
    Settings::save(_settings);

    Serial.print("Durdu. Pozisyon: %");
    Serial.println(_position);
    _updateLED();
}

// ─── Update Position ──────────────────────────────────
void CurtainController::_updatePosition() {
    if (_settings.totalPulses == 0) return;

    uint32_t pulses  = _encoder.getCount();
    uint8_t  delta   = map(pulses, 0, _settings.totalPulses, 0, 100);
    delta            = constrain(delta, 0, 100);

    if (_state == STATE_OPENING) {
        _position = constrain(_position + delta, 0, 100);
    } else if (_state == STATE_CLOSING) {
        _position = constrain(_position - delta, 0, 100);
    }
}

// ─── Update LED ───────────────────────────────────────
void CurtainController::_updateLED() {
    if (!_initialized) {
        _led.setUnknown();   // Turuncu sabit
        return;
    }

    switch (_state) {
        case STATE_IDLE:
#if CURTAIN_MODE == MODE_VERTICAL
            // Dikey perde: 0% ve 100% kapali
            if (_position <= 5 || _position >= 95)
                _led.setColor(CRGB::Red);      // Kapali
            else if (_position >= 45 && _position <= 55)
                _led.setColor(CRGB::Green);    // Tam acik
            else
                _led.setColor(CRGB::Yellow);   // Ara poz
#else
            if (_position >= 95)
                _led.setIdle();                // Tam acik yesil
            else if (_position <= 5)
                _led.setColor(CRGB::Red);      // Tam kapali kirmizi
            else
                _led.setColor(CRGB::Yellow);   // Ara poz sari
#endif
            break;
        case STATE_OPENING:  _led.setOpening(); break;
        case STATE_CLOSING:  _led.setClosing(); break;
        case STATE_UNKNOWN:  _led.setUnknown(); break;
        default: break;
    }
}

// ─── Helpers ──────────────────────────────────────────
uint8_t CurtainController::_getOpenTarget() {
#if CURTAIN_MODE == MODE_VERTICAL
    return 50;   // Orta = tam acik
#else
    return 100;
#endif
}

uint8_t CurtainController::_getCloseTarget() {
#if CURTAIN_MODE == MODE_VERTICAL
    return (_position <= 50) ? 0 : 100;  // En yakin kapali tarafa git
#else
    return 0;
#endif
}

uint8_t CurtainController::getPosition()    { return _position; }
CurtainState CurtainController::getState()  { return _state;    }