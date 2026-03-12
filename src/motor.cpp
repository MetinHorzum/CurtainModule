#include "Motor.h"

// ============================================================
//  Kurucu — Pin numaralarını kaydet
// ============================================================
Motor::Motor(uint8_t pin_ain1, uint8_t pin_ain2,
             uint8_t pin_nsleep, uint8_t pin_nfault) {
    _ain1   = pin_ain1;
    _ain2   = pin_ain2;
    _nsleep = pin_nsleep;
    _nfault = pin_nfault;
}

// ============================================================
//  begin() — Pinleri ayarla, sürücüyü uyandır
// ============================================================
void Motor::begin() {
    pinMode(_ain1,   OUTPUT);
    pinMode(_ain2,   OUTPUT);
    pinMode(_nsleep, OUTPUT);
    pinMode(_nfault, INPUT);   // NFAULT = giriş (sürücü bize söyler)

    coast();   // Başlangıçta serbest
    wake();    // Sürücüyü aktif et
}

// ============================================================
//  forward() — İleri git
// ============================================================
void Motor::forward(uint8_t speed) {
    wake();
    analogWrite(_ain1, speed);   // PWM
    analogWrite(_ain2, 0);       // Sabit LOW
}

// ============================================================
//  backward() — Geri git
// ============================================================
void Motor::backward(uint8_t speed) {
    wake();
    analogWrite(_ain1, 0);       // Sabit LOW
    analogWrite(_ain2, speed);   // PWM
}

// ============================================================
//  brake() — Ani fren
// ============================================================
void Motor::brake() {
    analogWrite(_ain1, 255);
    analogWrite(_ain2, 255);
}

// ============================================================
//  coast() — Serbest bırak (motor kendi durur)
// ============================================================
void Motor::coast() {
    analogWrite(_ain1, 0);
    analogWrite(_ain2, 0);
}

// ============================================================
//  sleep() — Sürücüyü uyut (güç tasarrufu)
// ============================================================
void Motor::sleep() {
    digitalWrite(_nsleep, LOW);
}

// ============================================================
//  wake() — Sürücüyü uyandır
// ============================================================
void Motor::wake() {
    digitalWrite(_nsleep, HIGH);
}

// ============================================================
//  isFault() — Hata kontrolü
// ============================================================
bool Motor::isFault() {
    return (digitalRead(_nfault) == LOW);  // LOW = hata var
}