#pragma once
#include <Arduino.h>
#include <EEPROM.h>

struct CurtainSettings {
    uint32_t totalPulses;       // Tam acik-kapali arasi pulse
    uint8_t  currentPosition;   // Son bilinen pozisyon (0=Kapali, 100=Acik)
    uint32_t closingTime_ms;    // Kapanma suresi
    uint32_t openingTime_ms;    // Acilma suresi
    bool     isInitialized;     // Init yapildi mi?
    uint8_t  checksum;          // Veri dogrulama
};

class Settings {
public:
    static void save(CurtainSettings& s);
    static bool load(CurtainSettings& s);
    static void reset();
    static void print(CurtainSettings& s);  // Debug icin

private:
    static const uint16_t EEPROM_ADDR = 0;
    static uint8_t calcChecksum(CurtainSettings& s);
};