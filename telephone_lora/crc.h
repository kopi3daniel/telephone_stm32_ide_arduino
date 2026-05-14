#ifndef CRC_H
#define CRC_H

#include <Arduino.h>
#include "config.h"

class CRCChecksum {  // <-- Renommé
    
public:
    static uint8_t crc8(const uint8_t* data, uint16_t length);
    static uint8_t crc8WithInit(const uint8_t* data, uint16_t length, uint8_t initValue);
    static bool verifyCRC8(const uint8_t* data, uint16_t length, uint8_t expectedCRC);
    
    static uint16_t crc16(const uint8_t* data, uint16_t length);
    static uint16_t crc16WithInit(const uint8_t* data, uint16_t length, uint16_t initValue);
    static bool verifyCRC16(const uint8_t* data, uint16_t length, uint16_t expectedCRC);
    static uint16_t appendCRC16(uint8_t* data, uint16_t length);
    
    static uint8_t xorChecksum(const uint8_t* data, uint16_t length);
    static bool verifyXOR(const uint8_t* data, uint16_t length, uint8_t expectedChecksum);
    
    static uint16_t fletcher16(const uint8_t* data, uint16_t length);
    static bool verifyFletcher16(const uint8_t* data, uint16_t length, uint16_t expectedSum);
    
    static uint16_t autoChecksum(const uint8_t* data, uint16_t length);
    static bool verifyAutoChecksum(const uint8_t* data, uint16_t length, uint16_t checksum);
    static uint16_t appendChecksum(uint8_t* data, uint16_t length);
    
    static bool verifyLoRaPacket(const uint8_t* packet, uint16_t length);
    static uint8_t contactChecksum(const char* name, const char* number);
    static uint16_t smsChecksum(const char* text, uint16_t length);
    static bool verifySettings(const uint8_t* settingsData, uint16_t length);
    
    static bool selfTest(void);
    static const char* getVersion(void);
    static uint16_t stringChecksum(const char* str);
};

#endif // CRC_H