#include "Settings.h"

void Settings::save(CurtainSettings& s) {
    s.checksum = calcChecksum(s);
    EEPROM.put(EEPROM_ADDR, s);
}

bool Settings::load(CurtainSettings& s) {
    EEPROM.get(EEPROM_ADDR, s);
    if (s.checksum != calcChecksum(s)) return false;
    if (!s.isInitialized)              return false;
    return true;
}

void Settings::reset() {
    CurtainSettings empty = {0, 0, false, 0};
    EEPROM.put(EEPROM_ADDR, empty);
}

uint8_t Settings::calcChecksum(CurtainSettings& s) {
    uint8_t sum = 0;
    sum ^= (s.totalPulses >> 24) & 0xFF;
    sum ^= (s.totalPulses >> 16) & 0xFF;
    sum ^= (s.totalPulses >>  8) & 0xFF;
    sum ^= (s.totalPulses      ) & 0xFF;
    sum ^= s.lastPosition;
    sum ^= s.isInitialized ? 0x01 : 0x00;
    return sum;
}