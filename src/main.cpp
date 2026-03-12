#include <Arduino.h>
#include "config.h"
#include "Motor.h"
#include "Encoder.h"
#include "LED.h"
#include "Button.h"
#include "Settings.h"
#include "CurtainController.h"

Motor             motor(PIN_AIN1, PIN_AIN2, PIN_NSLEEP, PIN_NFAULT);
Encoder           encoder(PIN_ENCODER);
LED               led;
Button            button(PIN_BUTTON);
CurtainController curtain(motor, encoder, led, button);

void encoderISR() {
    encoder.update();
}

void setup() {
    Serial.begin(BAUD_RATE);
    Serial.println("CurtainModule v0.5");

    motor.begin();
    encoder.begin();
    led.begin();
    button.begin();

    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER),
                    encoderISR, FALLING);

    curtain.begin();
}

void loop() {
    curtain.update();

    // Debug: Her 2 sn pozisyon yaz
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint >= 2000) {
        Serial.print("Poz: %");
        Serial.println(curtain.getPosition());
        lastPrint = millis();
    }
}