#pragma once
#include <Arduino.h>

// ============================================================
//  Motor - DRV8833 Motor Sürücü Sınıfı
//  Pinler: AIN1, AIN2 (PWM), NSLEEP, NFAULT
// ============================================================

class Motor {
public:
    // --- Kurucu fonksiyon ---
    Motor(uint8_t pin_ain1, uint8_t pin_ain2,
          uint8_t pin_nsleep, uint8_t pin_nfault);

    void begin();           // Pin modlarını ayarla, sürücüyü uyandır
    void forward(uint8_t speed);   // İleri (0-255)
    void backward(uint8_t speed);  // Geri  (0-255)
    void brake();           // Ani dur (fren)
    void coast();           // Serbest bırak
    void sleep();           // Sürücüyü uyut
    void wake();            // Sürücüyü uyandır
    bool isFault();         // Hata var mı?

private:
    uint8_t _ain1;
    uint8_t _ain2;
    uint8_t _nsleep;
    uint8_t _nfault;
};