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

enum State { IDLE, MOVING, STOPPED_MID };  // STOPPED_MID = dönüm noktası arasında durdu
State state = IDLE;

#define POS_0    0
#define POS_90   50
#define POS_180  100

bool    goingForward = true;
uint8_t targetPos    = POS_0;
uint32_t targetPulse = 0;

void encoderISR() { encoder.update(); }

// ─────────────────────────────────────────
uint32_t positionToPulse(uint8_t position) {
    return (uint32_t)settings.totalPulses * position / 100;
}

bool isAtWaypoint(uint8_t pos) {
    return (pos == POS_0 || pos == POS_90 || pos == POS_180);
}

// ─────────────────────────────────────────
void startMove(uint8_t from, uint8_t target) {
    targetPos             = target;
    uint32_t fromPulse    = positionToPulse(from);
    targetPulse           = positionToPulse(target);

    encoder.resetCount();

    Serial.print("[MOT] "); Serial.print(from);
    Serial.print("% → ");   Serial.print(target); Serial.println("%");

    if (target > from) {
        goingForward = true;
        led.setOpening();
        motor.forward(60);
    } else {
        goingForward = false;
        led.setClosing();
        motor.backward(60);
    }

    state = MOVING;
}

// ─────────────────────────────────────────
uint8_t nextWaypoint(uint8_t current) {
    if      (current == POS_0)   return POS_90;
    else if (current == POS_90)  return goingForward ? POS_180 : POS_0;
    else                         return POS_90;  // 180 → 90
}

// ─────────────────────────────────────────
void runCalibration() {
    Serial.println("────────────────────────────���");
    Serial.println("KALIBRASYON BASLADI");
    Serial.println("─────────────────────────────");

    Serial.println("[1] Motor kapaniyor...");
    led.setClosing();
    encoder.resetCount();
    motor.forward(50);
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
        if (motor.isFault()) {
            motor.brake(); Serial.println("[!] MOTOR HATA!");
            led.setError(); while(true) { led.update(); }
        }
        if (encoder.isStalled(STALL_TIMEOUT_MS)) {
            motor.brake();
            settings.closingTime_ms = millis() - startTime;
            Serial.println("[1] OK - Tam kapali");
            break;
        }
    }

    delay(500);
    encoder.resetCount();

    Serial.println("[2] Motor aciliyor...");
    led.setOpening();
    motor.backward(50);
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
        if (motor.isFault()) {
            motor.brake(); Serial.println("[!] MOTOR HATA!");
            led.setError(); while(true) { led.update(); }
        }
        if (encoder.isStalled(STALL_TIMEOUT_MS)) {
            motor.brake();
            settings.openingTime_ms = millis() - startTime;
            Serial.println("[2] OK - Tam acik");
            Serial.print("    Toplam pulse: "); Serial.println(encoder.getCount());
            break;
        }
    }

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

    goingForward = false;  // 180'deyiz
    state = IDLE;
}

// ─────────────────────────────────────────
void setup() {
    Serial.begin(BAUD_RATE);
    Serial.println("═════════════════════════════");
    Serial.println("CurtainModule v1.0");
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
                // ── Hareket ediyorken → DUR ──
                motor.brake();

                // Anlık pozisyonu hesapla
                uint32_t travelled   = encoder.getCount();
                uint32_t currentP    = positionToPulse(settings.currentPosition);
                uint32_t newPulse    = goingForward
                                     ? currentP + travelled
                                     : (currentP > travelled ? currentP - travelled : 0);
                uint8_t  newPos      = (uint8_t)(newPulse * 100 / settings.totalPulses);

                settings.currentPosition = newPos;
                Settings::save(settings);

                Serial.print("[MOT] DURDURULDU | Pozisyon: ~");
                Serial.print(newPos); Serial.println("%");

                state = STOPPED_MID;  // Dönüm noktası arasında durdu
                led.setIdle();

            } else if (state == STOPPED_MID) {
                // ── Arada duruyorken → Geri dön ──
                goingForward = !goingForward;
                uint8_t backTarget = goingForward ? POS_90 : POS_90;

                // Hangi dönüm noktasına dönecek?
                backTarget = goingForward
                           ? POS_90   // 0→90 arasındayken geri = 0'a dön
                           : POS_90;  // 90→180 arasındayken geri = 90'a dön

                // Tam olarak: ters yöndeki en yakın waypoint
                backTarget = goingForward ? POS_0 : POS_180;
                // Ama nerede durduğumuza göre:
                uint8_t cur = settings.currentPosition;
                if (cur < POS_90) {
                    backTarget = goingForward ? POS_90 : POS_0;
                } else {
                    backTarget = goingForward ? POS_180 : POS_90;
                }

                Serial.println("[MOT] Ters yone donuluyor...");
                startMove(settings.currentPosition, backTarget);

            } else {
                // ── IDLE (dönüm noktasında) → Normal senaryo ──
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

                // Dönüm noktasına varıldı → IDLE
                if      (targetPos == POS_0)   goingForward = true;
                else if (targetPos == POS_180)  goingForward = false;
                // POS_90'a varınca yön değişmez, geldiği yönü korur

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