/**
 * ---------------------------------------------------------------------------
 * crc.cpp - Implementation des fonctions de checksum
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "crc.h"
#include <string.h>

// ============================================
// TABLE CRC8 (polynome 0x07)
// ============================================
static const uint8_t crc8_table[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

// ============================================
// TABLE CRC16-CCITT (polynome 0x1021)
// ============================================
static const uint16_t crc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0AB0, 0x3A93, 0x2A82,
    0xDBED, 0xCBCC, 0xFBAF, 0xEB8E, 0x9B69, 0x8B48, 0xBB2B, 0xAB0A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78
};

// ============================================
// CRC8
// ============================================
uint8_t CRCChecksum::crc8(const uint8_t* data, uint16_t length) {
    return crc8WithInit(data, length, 0x00);
}

uint8_t CRCChecksum::crc8WithInit(const uint8_t* data, uint16_t length, uint8_t initValue) {
    uint8_t crc = initValue;
    for (uint16_t i = 0; i < length; i++) {
        crc = crc8_table[crc ^ data[i]];
    }
    return crc;
}

bool CRCChecksum::verifyCRC8(const uint8_t* data, uint16_t length, uint8_t expectedCRC) {
    return (crc8(data, length) == expectedCRC);
}

// ============================================
// CRC16-CCITT
// ============================================
uint16_t CRCChecksum::crc16(const uint8_t* data, uint16_t length) {
    return crc16WithInit(data, length, 0xFFFF);
}

uint16_t CRCChecksum::crc16WithInit(const uint8_t* data, uint16_t length, uint16_t initValue) {
    uint16_t crc = initValue;
    for (uint16_t i = 0; i < length; i++) {
        crc = (crc << 8) ^ crc16_table[((crc >> 8) ^ data[i]) & 0xFF];
    }
    return crc;
}

bool CRCChecksum::verifyCRC16(const uint8_t* data, uint16_t length, uint16_t expectedCRC) {
    return (crc16(data, length) == expectedCRC);
}

uint16_t CRCChecksum::appendCRC16(uint8_t* data, uint16_t length) {
    uint16_t crc = crc16(data, length);
    data[length] = crc & 0xFF;
    data[length + 1] = (crc >> 8) & 0xFF;
    return length + 2;
}

// ============================================
// XOR CHECKSUM
// ============================================
uint8_t CRCChecksum::xorChecksum(const uint8_t* data, uint16_t length) {
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

bool CRCChecksum::verifyXOR(const uint8_t* data, uint16_t length, uint8_t expectedChecksum) {
    return (xorChecksum(data, length) == expectedChecksum);
}

// ============================================
// FLETCHER-16
// ============================================
uint16_t CRCChecksum::fletcher16(const uint8_t* data, uint16_t length) {
    uint8_t sum1 = 0;
    uint8_t sum2 = 0;
    for (uint16_t i = 0; i < length; i++) {
        sum1 += data[i];
        sum2 += sum1;
    }
    return (sum2 << 8) | sum1;
}

bool CRCChecksum::verifyFletcher16(const uint8_t* data, uint16_t length, uint16_t expectedSum) {
    return (fletcher16(data, length) == expectedSum);
}

// ============================================
// CHECKSUM AUTOMATIQUE
// ============================================
uint16_t CRCChecksum::autoChecksum(const uint8_t* data, uint16_t length) {
    if (length < 16) {
        return xorChecksum(data, length);
    } else if (length < 256) {
        return crc8(data, length);
    } else {
        return crc16(data, length);
    }
}

bool CRCChecksum::verifyAutoChecksum(const uint8_t* data, uint16_t length, uint16_t checksum) {
    return (autoChecksum(data, length) == checksum);
}

uint16_t CRCChecksum::appendChecksum(uint8_t* data, uint16_t length) {
    uint16_t checksum = autoChecksum(data, length);
    if (length < 16) {
        data[length] = checksum & 0xFF;
        return length + 1;
    } else if (length < 256) {
        data[length] = checksum & 0xFF;
        return length + 1;
    } else {
        data[length] = checksum & 0xFF;
        data[length + 1] = (checksum >> 8) & 0xFF;
        return length + 2;
    }
}

// ============================================
// FONCTIONS SPECIFIQUES AU PROJET
// ============================================
bool CRCChecksum::verifyLoRaPacket(const uint8_t* packet, uint16_t length) {
    if (packet == NULL || length < 7) return false;
    if (packet[0] != PKT_SYNC_BYTE) return false;
    uint8_t payloadSize = packet[6];
    if (payloadSize > PKT_MAX_PAYLOAD) return false;
    if (length < (uint16_t)(PKT_HEADER_SIZE + payloadSize)) return false;
    return true;
}

uint8_t CRCChecksum::contactChecksum(const char* name, const char* number) {
    uint8_t buffer[48];
    memset(buffer, 0, sizeof(buffer));
    if (name) strncpy((char*)buffer, name, 31);
    if (number) strncpy((char*)buffer + 32, number, 15);
    return crc8(buffer, 48);
}

uint16_t CRCChecksum::smsChecksum(const char* text, uint16_t length) {
    if (text == NULL || length == 0) return 0;
    return crc16((const uint8_t*)text, length);
}

bool CRCChecksum::verifySettings(const uint8_t* settingsData, uint16_t length) {
    if (settingsData == NULL || length < 4) return false;
    uint8_t storedChecksum = settingsData[length - 1];
    uint8_t calculated = crc8(settingsData, length - 1);
    return (storedChecksum == calculated);
}

// ============================================
// AUTO-TEST
// ============================================
bool CRCChecksum::selfTest(void) {
    const uint8_t testData[] = {0x31, 0x32, 0x33, 0x34, 0x35};  // "12345"
    const uint8_t testLength = 5;
    
    // CRC8 de "12345" = 0xBC
    if (crc8(testData, testLength) != 0xBC) return false;
    
    // XOR de "12345" = 0x07
    if (xorChecksum(testData, testLength) != 0x07) return false;
    
    // Fletcher-16 ne doit pas etre nul
    uint16_t fletcher = fletcher16(testData, testLength);
    if (fletcher == 0) return false;
    
    return true;
}

const char* CRCChecksum::getVersion(void) { return "CRCLib v1.0"; }

uint16_t CRCChecksum::stringChecksum(const char* str) {
    if (str == NULL) return 0;
    return crc16((const uint8_t*)str, strlen(str));
}

// ============================================
// FIN DU FICHIER crc.cpp
// ============================================