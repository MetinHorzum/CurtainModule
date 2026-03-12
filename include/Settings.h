#pragma once
#include <Arduino.h>
#include <EEPROM.h>

struct CurtainSettings {
    uint32_t totalPulses;
    uint8_t  lastPosition;    // Son bilinen pozisyon
    bool     isInitialized;
    uint8_t  checksum;
};

class Settings {
public:
    static void save(CurtainSettings& s);
    static bool load(CurtainSettings& s);
    static void reset();

private:
    static const uint16_t EEPROM_ADDR = 0;
    static uint8_t calcChecksum(CurtainSettings& s);
};