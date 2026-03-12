#include <Arduino.h>
#include "config.h"
#include "Motor.h"

// Motor nesnesi config.h pinleri
Motor motor(PIN_AIN1, PIN_AIN2, PIN_NSLEEP, PIN_NFAULT);

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println(F("CurtainModule v0.1 - Basliyor..."));

  motor.begin();
  Serial.println(F("Motor hazir."));
}

void loop() {
    // Test: İleri 2 sn → Dur 1 sn → Geri 2 sn → Dur 1 sn
    
    Serial.println(F("İleri..."));
    motor.forward(200);   // %78 hız
    delay(2000);

    Serial.println(F("Fren!"));
    motor.brake();
    delay(1000);

    Serial.println(F("Geri..."));
    motor.backward(200);
    delay(2000);

    Serial.println(F("Fren!"));
    motor.brake();
    delay(1000);

    // Hata kontrolü
    if (motor.isFault()) {
        Serial.println(F("⚠️ MOTOR HATA!"));
        motor.sleep();
        while(true);   // Dur, reset bekle
    }
}
