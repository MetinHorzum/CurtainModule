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
    CurtainSettings empty = {0, 0, 0, 0, false, 0};
    EEPROM.put(EEPROM_ADDR, empty);
}

void Settings::print(CurtainSettings& s) {
    Serial.println("─────────────────────────────");
    Serial.println("[EEPROM] Kayitli Ayarlar:");
    Serial.print("  totalPulses     : "); Serial.println(s.totalPulses);
    Serial.print("  currentPosition : "); Serial.print(s.currentPosition); Serial.println("%");
    Serial.print("  closingTime_ms  : "); Serial.print(s.closingTime_ms); Serial.println("ms");
    Serial.print("  openingTime_ms  : "); Serial.print(s.openingTime_ms); Serial.println("ms");
    Serial.print("  isInitialized   : "); Serial.println(s.isInitialized ? "EVET" : "HAYIR");
    Serial.println("─────────────────────────────");
}

uint8_t Settings::calcChecksum(CurtainSettings& s) {
    uint8_t sum = 0;
    sum ^= (s.totalPulses >> 24) & 0xFF;
    sum ^= (s.totalPulses >> 16) & 0xFF;
    sum ^= (s.totalPulses >>  8) & 0xFF;
    sum ^= (s.totalPulses      ) & 0xFF;
    sum ^= s.currentPosition;
    sum ^= (s.closingTime_ms >> 24) & 0xFF;
    sum ^= (s.closingTime_ms >> 16) & 0xFF;
    sum ^= (s.closingTime_ms >>  8) & 0xFF;
    sum ^= (s.closingTime_ms      ) & 0xFF;
    sum ^= (s.openingTime_ms >> 24) & 0xFF;
    sum ^= (s.openingTime_ms >> 16) & 0xFF;
    sum ^= (s.openingTime_ms >>  8) & 0xFF;
    sum ^= (s.openingTime_ms      ) & 0xFF;
    sum ^= s.isInitialized ? 0x01 : 0x00;
    return sum;
}