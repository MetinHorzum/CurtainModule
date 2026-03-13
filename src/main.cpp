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
    Serial.println("KALIBRASYON BASLADI");
    Serial.println("─────────────────────────────");

    // ADIM 1: Kapan
    Serial.println("[1] Motor kapaniyor (PWM=50)...");
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
            Serial.println("[!] MOTOR HATA!");
            led.setError();
            while(true) { led.update(); }
        }

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
    Serial.println("[2] Motor aciliyor (PWM=50)...");
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
            Serial.println("[!] MOTOR HATA!");
            led.setError();
            while(true) { led.update(); }
        }

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
    settings.currentPosition = 100;
    settings.isInitialized   = true;
    Settings::save(settings);

    // ADIM 4: Dogrula
    CurtainSettings verify;
    if (Settings::load(verify)) {
        Serial.println("[3] OK - EEPROM kaydedildi");
        Settings::print(verify);
    } else {
        Serial.println("[3] HATA - EEPROM basarisiz!");
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
    Serial.println("CurtainModule v0.7");
    Serial.println("═════════════════════════════");

    motor.begin();  Serial.println("[INIT] Motor OK");
    encoder.begin();
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER), encoderISR, FALLING);
                    Serial.println("[INIT] Encoder OK");
    led.begin();    Serial.println("[INIT] LED OK");
    button.begin(); Serial.println("[INIT] Buton OK");

    Serial.println("─────────────────────────────");

    // EEPROM kontrol
    if (!Settings::load(settings)) {
        Serial.println("[EEPROM] Kayit yok!");
        Serial.println("Kalibrasyon icin 3sn basin...");
        led.setInit();  // Sari hizli
    } else {
        Serial.println("[EEPROM] Ayarlar yuklendi:");
        Settings::print(settings);
        Serial.println("Hazir. | Sifirlamak icin 5sn basin.");
        led.setIdle();  // Yesil sabit
    }
}

// ─────────────────────────────────────────
void loop() {
    button.update();
    led.update();

    if (settings.isInitialized) {
        // EEPROM dolu → 5sn basili = Sifirla
        if (button.isLongPress(5000)) {
            Serial.println("─────────────────────────────");
            Serial.println("EEPROM siliniyor...");
            Settings::reset();
            settings.isInitialized = false;
            Serial.println("Silindi! Kalibrasyon icin 3sn basin.");
            led.setInit();
        }
    } else {
        // EEPROM bos → 3sn basili = Kalibrasyon
        if (button.isLongPress(3000)) {
            runCalibration();
            Serial.println("Hazir. | Sifirlamak icin 5sn basin.");
        }
    }
}