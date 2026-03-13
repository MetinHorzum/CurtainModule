#include <Arduino.h>
#include "config.h"
#include "Motor.h"
#include "Encoder.h"
#include "LED.h"
#include "Button.h"
#include "Settings.h"

Motor   motor(PIN_AIN1, PIN_AIN2, PIN_NSLEEP, PIN_NFAULT);
Encoder encoder(PIN_ENCODER);
LED     led;
Button  button(PIN_BUTTON);

CurtainSettings settings;

enum State { IDLE, MOVING, STOPPED_MID };
State state = IDLE;

#define POS_0    0
#define POS_90   50
#define POS_180  100

bool     goingForward = true;
uint8_t  targetPos    = POS_0;
uint32_t targetPulse  = 0;
uint32_t moveStartTime = 0;   // Timeout için

void encoderISR() { encoder.update(); }

// ─────────────────────────────────────────
uint32_t positionToPulse(uint8_t position) {
    return (uint32_t)settings.totalPulses * position / 100;
}

bool isAtWaypoint(uint8_t pos) {
    return (pos == POS_0 || pos == POS_90 || pos == POS_180);
}

// ─────────────────────────────────────────
void handleFault() {
    motor.brake();
    Serial.println("─────────────────────────────");
    Serial.println("[!] MOTOR FAULT - Overcurrent/Kisa devre!");
    Serial.println("[!] Sistem durduruldu.");
    Serial.println("─────────────────────────────");
    state = IDLE;
    led.setError();
}

// ─────────────────────────────────────────
void handleTimeout() {
    motor.brake();
    Serial.println("─────────────────────────────");
    Serial.println("[!] HAREKET TIMEOUT!");
    Serial.print("[!] "); Serial.print(MOVE_TIMEOUT_MS / 1000);
    Serial.println("sn icinde hedefe ulasilamadi.");
    Serial.println("[!] Sistem durduruldu.");
    Serial.println("─────────────────────────────");

    // Anlık pozisyonu tahmin et
    uint32_t travelled = encoder.getCount();
    uint32_t currentP  = positionToPulse(settings.currentPosition);
    uint32_t newPulse  = goingForward
                       ? currentP + travelled
                       : (currentP > travelled ? currentP - travelled : 0);
    uint8_t newPos     = (uint8_t)(newPulse * 100 / settings.totalPulses);

    settings.currentPosition = newPos;
    Settings::save(settings);

    Serial.print("[!] Son bilinen pozisyon: ~");
    Serial.print(newPos); Serial.println("%");

    state = STOPPED_MID;
    led.setError();
    delay(2000);
    led.setIdle();
}

// ─────────────────────────────────────────
void startMove(uint8_t from, uint8_t target) {
    targetPos    = target;
    targetPulse  = positionToPulse(target);
    moveStartTime = millis();

    encoder.resetCount();

    Serial.print("[MOT] "); Serial.print(from);
    Serial.print("% → ");   Serial.print(target); Serial.println("%");
    Serial.print("    PWM: "); Serial.println(PWM_NORMAL);

    if (target > from) {
        goingForward = true;
        led.setOpening();
        motor.forward(PWM_NORMAL);
    } else {
        goingForward = false;
        led.setClosing();
        motor.backward(PWM_NORMAL);
    }

    state = MOVING;
}

// ─────────────────────────────────────────
uint8_t nextWaypoint(uint8_t current) {
    if      (current == POS_0)   return POS_90;
    else if (current == POS_90)  return goingForward ? POS_180 : POS_0;
    else                         return POS_90;
}

// ─────────────────────────────────────────
void runCalibration() {
    Serial.println("─────────────────────────────");
    Serial.println("KALIBRASYON BASLADI");
    Serial.println("─────────────────────────────");
    Serial.print("    PWM: "); Serial.println(PWM_CALIBRATION);

    // ADIM 1: Kapan
    Serial.println("[1] Motor kapaniyor...");
    led.setClosing();
    encoder.resetCount();
    motor.forward(PWM_CALIBRATION);

    uint32_t startTime = millis();
    while (true) {
        led.update();
        static uint32_t lp = 0;
        if (millis() - lp > 200) {
            Serial.print("    Pulse: "); Serial.print(encoder.getCount());
            Serial.print(" | Sure: ");  Serial.print(millis() - startTime);
            Serial.println("ms");
            lp = millis();
        }
        if (motor.isFault()) { handleFault(); return; }
        if (encoder.isStalled(STALL_TIMEOUT_MS)) {
            motor.brake();
            settings.closingTime_ms = millis() - startTime;
            Serial.println("[1] OK - Tam kapali");
            Serial.print("    Sure: "); Serial.print(settings.closingTime_ms); Serial.println("ms");
            break;
        }
    }

    delay(500);
    encoder.resetCount();

    // ADIM 2: Ac
    Serial.println("[2] Motor aciliyor...");
    led.setOpening();
    motor.backward(PWM_CALIBRATION);

    startTime = millis();
    while (true) {
        led.update();
        static uint32_t lp2 = 0;
        if (millis() - lp2 > 200) {
            Serial.print("    Pulse: "); Serial.print(encoder.getCount());
            Serial.print(" | Sure: ");  Serial.print(millis() - startTime);
            Serial.println("ms");
            lp2 = millis();
        }
        if (motor.isFault()) { handleFault(); return; }
        if (encoder.isStalled(STALL_TIMEOUT_MS)) {
            motor.brake();
            settings.openingTime_ms = millis() - startTime;
            Serial.println("[2] OK - Tam acik");
            Serial.print("    Sure: ");        Serial.print(settings.openingTime_ms); Serial.println("ms");
            Serial.print("    Toplam pulse: "); Serial.println(encoder.getCount());
            break;
        }
    }

    // ADIM 3: Kaydet
    settings.totalPulses     = encoder.getCount();
    settings.currentPosition = POS_180;
    settings.isInitialized   = true;
    Settings::save(settings);

    CurtainSettings verify;
    if (Settings::load(verify)) {
        Serial.println("[3] OK - EEPROM kaydedildi");
        Settings::print(verify);
    } else {
        Serial.println("[3] HATA!"); led.setError();
        while(true) { led.update(); }
    }

    Serial.println("KALIBRASYON TAMAMLANDI ✓");
    led.setSaved(); delay(1500); led.setIdle();

    goingForward = false;
    state = IDLE;
}

// ─────────────────────────────────────────
void setup() {
    Serial.begin(BAUD_RATE);
    Serial.println("═════════════════════════════");
    Serial.println("CurtainModule v1.1");
    Serial.println("═════════════════════════════");

    motor.begin();   Serial.println("[INIT] Motor OK");
    encoder.begin();
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER), encoderISR, FALLING);
                     Serial.println("[INIT] Encoder OK");
    led.begin();     Serial.println("[INIT] LED OK");
    button.begin();  Serial.println("[INIT] Buton OK");

    Serial.println("─────────────────────────────");

    if (!Settings::load(settings)) {
        Serial.println("[EEPROM] Kayit yok!");
        Serial.println("Kalibrasyon icin 3sn basin...");
        led.setInit();
    } else {
        Serial.println("[EEPROM] Ayarlar yuklendi:");
        Settings::print(settings);
        Serial.println("Hazir. | Sifirlamak icin 5sn basin.");
        led.setIdle();
        goingForward = (settings.currentPosition <= POS_90);
    }
}

// ─────────────────────────────────────────
void loop() {
    button.update();
    led.update();

    if (settings.isInitialized) {

        // ── NFAULT kontrolü (her zaman aktif) ──
        if (motor.isFault() && state == MOVING) {
            handleFault();
            return;
        }

        // ── Timeout kontrolü ──
        if (state == MOVING) {
            if (millis() - moveStartTime >= MOVE_TIMEOUT_MS) {
                handleTimeout();
                return;
            }
        }

        // 5sn → Sifirla
        if (button.isLongPress(5000)) {
            motor.brake();
            Settings::reset();
            settings.isInitialized = false;
            state = IDLE;
            Serial.println("EEPROM silindi! Kalibrasyon icin 3sn basin.");
            led.setInit();
            return;
        }

        // Kisa bas
        if (button.isShortPress()) {
            if (state == MOVING) {
                // Dur
                motor.brake();
                uint32_t travelled = encoder.getCount();
                uint32_t currentP  = positionToPulse(settings.currentPosition);
                uint32_t newPulse  = goingForward
                                   ? currentP + travelled
                                   : (currentP > travelled ? currentP - travelled : 0);
                uint8_t newPos     = (uint8_t)(newPulse * 100 / settings.totalPulses);

                settings.currentPosition = newPos;
                Settings::save(settings);

                Serial.print("[MOT] DURDURULDU | Pozisyon: ~");
                Serial.print(newPos); Serial.println("%");

                state = STOPPED_MID;
                led.setIdle();

            } else if (state == STOPPED_MID) {
                // Ters yöne dön
                goingForward      = !goingForward;
                uint8_t cur       = settings.currentPosition;
                uint8_t backTarget = (cur < POS_90)
                                   ? (goingForward ? POS_90 : POS_0)
                                   : (goingForward ? POS_180 : POS_90);

                Serial.println("[MOT] Ters yone donuluyor...");
                startMove(settings.currentPosition, backTarget);

            } else {
                // IDLE → normal senaryo
                uint8_t target = nextWaypoint(settings.currentPosition);
                startMove(settings.currentPosition, target);
            }
        }

        // ── Hareket takibi ──
        if (state == MOVING) {
            uint32_t travelled = encoder.getCount();
            uint32_t fromPulse = positionToPulse(settings.currentPosition);
            uint32_t pulseDiff = (targetPulse > fromPulse)
                               ? (targetPulse - fromPulse)
                               : (fromPulse - targetPulse);

            if (travelled >= pulseDiff) {
                motor.brake();
                settings.currentPosition = targetPos;
                Settings::save(settings);

                Serial.print("[MOT] Hedefe ulasildi: ");
                Serial.print(targetPos); Serial.println("%");

                if      (targetPos == POS_0)   goingForward = true;
                else if (targetPos == POS_180)  goingForward = false;

                state = IDLE;
                led.setIdle();
            }
        }

    } else {
        // 3sn → Kalibrasyon
        if (button.isLongPress(3000)) {
            runCalibration();
            Serial.println("Hazir. | Sifirlamak icin 5sn basin.");
        }
    }
}