#pragma once

// ============================================================
//  CurtainModule - Pin & Sabit Tanımları
//  Kart: ATmega328P Custom PCB
//  Tarih: 2026-03-12
// ============================================================

// --- Motor (DRV8833) ---
#define PIN_AIN1    11   // Motor A giriş 1 (PWM)
#define PIN_AIN2    6    // Motor A giriş 2 (PWM)
#define PIN_NSLEEP  3    // DRV8833 uyku/aktif pin (HIGH = aktif)
#define PIN_NFAULT  A2   // DRV8833 hata çıkışı (LOW = hata var)

// --- Enkoder ---
#define PIN_ENCODER 2    // Enkoder sinyal (INT0 - interrupt)

// --- Kullanıcı Arayüzü ---
#define PIN_BUTTON  4    // Buton girişi
#define PIN_LED     5    // IC LED çıkışı (PWM capable)

// --- RS485 ---
#define PIN_RS485_EN  7  // RS485 yön kontrolü (TX=HIGH, RX=LOW)
#define PIN_RS485_RO  8  // RS485 alıcı çıkışı (RO)
#define PIN_RS485_DI  9  // RS485 sürücü girişi (DI)

// --- Sistem Sabitleri ---
#define BAUD_RATE       115200
#define MOTOR_PWM_FREQ  1000    // Hz (ileride TIMER ayarı için)
#define DEBOUNCE_MS     50      // Buton debounce süresi (ms)