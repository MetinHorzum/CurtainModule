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

void encoderISR() {
    encoder.update();
}

// ─────────────────────────────────────────
void runCalibration() {
    Serial.println("─────────────────────────────");
    Serial.println("KALIBRASYON MODU BASLADI");
    Serial.println("─────────────────────────────");
    led.setInit();

    // ADIM 1: Buton bekle
    Serial.println("[1] Butona bas -> Baslat");
    while (!button.isShortPress()) {
        button.update();
        led.update();
    }
    Serial.println("[1] OK - Buton algilandi");
    delay(200);

    // ADIM 2: Kapan → Stall = Kapali nokta
    Serial.println("[2] Motor kapaniyor (PWM=50)...");
    led.setClosing();
    encoder.resetCount();
    motor.forward(50);

    uint32_t startTime = millis();
    while (true) {
        led.update();

        static uint32_t lastPrint = 0;
        if (millis() - lastPrint > 200) {
            Serial.print("    Pulse: "); Serial.print(encoder.getCount());
            Serial.print(" | Sure: ");  Serial.print(millis() - startTime);
            Serial.println("ms");
            lastPrint = millis();
        }

        if (motor.isFault()) {
            motor.brake();
            Serial.println("[!] MOTOR HATA - NFAULT tetiklendi!");
            led.setError();
            while(true) { led.update(); }
        }

        if (encoder.isStalled(STALL_TIMEOUT_MS)) {
            motor.brake();
            settings.closingTime_ms = millis() - startTime;
            Serial.println("[2] OK - Stall tespit edildi (Tam kapali)");
            Serial.print("    Kapanma suresi: "); Serial.print(settings.closingTime_ms); Serial.println("ms");
            break;
        }
    }

    delay(500);
    encoder.resetCount();

    // ADIM 3: Ac → Stall = Acik nokta
    Serial.println("[3] Motor aciliyor (PWM=50)...");
    led.setOpening();
    motor.backward(50);

    startTime = millis();
    while (true) {
        led.update();

        static uint32_t lastPrint2 = 0;
        if (millis() - lastPrint2 > 200) {
            Serial.print("    Pulse: "); Serial.print(encoder.getCount());
            Serial.print(" | Sure: ");  Serial.print(millis() - startTime);
            Serial.println("ms");
            lastPrint2 = millis();
        }

        if (motor.isFault()) {
            motor.brake();
            Serial.println("[!] MOTOR HATA - NFAULT tetiklendi!");
            led.setError();
            while(true) { led.update(); }
        }

        if (encoder.isStalled(STALL_TIMEOUT_MS)) {
            motor.brake();
            settings.openingTime_ms = millis() - startTime;
            Serial.println("[3] OK - Stall tespit edildi (Tam acik)");
            Serial.print("    Acilma suresi: "); Serial.print(settings.openingTime_ms); Serial.println("ms");
            Serial.print("    Toplam pulse:  "); Serial.println(encoder.getCount());
            break;
        }
    }

    // ADIM 4: Kaydet
    settings.totalPulses     = encoder.getCount();
    settings.currentPosition = 100;   // Simdi tam acik
    settings.isInitialized   = true;
    Settings::save(settings);

    // ADIM 5: Dogrula
    CurtainSettings verify;
    if (Settings::load(verify)) {
        Serial.println("[4] OK - EEPROM dogrulama basarili");
        Settings::print(verify);
    } else {
        Serial.println("[4] HATA - EEPROM dogrulama basarisiz!");
        led.setError();
        while(true) { led.update(); }
    }

    Serial.println("KALIBRASYON TAMAMLANDI ✓");
    Serial.println("─────────────────────────────");
    led.setSaved();
    delay(1500);
    led.setIdle();
}

// ─────────────────────────────────────────
void setup() {
    Serial.begin(BAUD_RATE);
    Serial.println("═════════════════════════════");
    Serial.println("CurtainModule v0.6");
    Serial.println("═════════════════════════════");

    motor.begin();
    Serial.println("[INIT] Motor OK");

    encoder.begin();
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER), encoderISR, FALLING);
    Serial.println("[INIT] Encoder OK");

    led.begin();
    Serial.println("[INIT] LED OK");

    button.begin();
    Serial.println("[INIT] Buton OK");

    Serial.println("─────────────────────────────");

    // EEPROM kontrol
    if (!Settings::load(settings)) {
        Serial.println("[EEPROM] Kayit yok → Kalibrasyon gerekli!");
        led.setInit();
    } else {
        Serial.println("[EEPROM] Ayarlar yuklendi:");
        Settings::print(settings);
        led.setIdle();
    }

    Serial.println("Hazir.");
    Serial.println("Kisa bas → Kalibrasyon");
}

// ─────────────────────────────────────────
void loop() {
    button.update();
    led.update();

    if (button.isShortPress()) {
        runCalibration();
        Serial.println("Hazir.");
        Serial.println("Kisa bas → Kalibrasyon");
    }
}