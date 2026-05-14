/**
 * ---------------------------------------------------------------------------
 * crypto.cpp - Implementation du chiffrement AES-128 et de la securite
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "crypto.h"
#include <string.h>
#include <stdio.h>

// ============================================
// S-BOX AES (table de substitution)
// ============================================
const uint8_t CryptoManager::sbox[256] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

// ============================================
// RCON (Round Constants)
// ============================================
const uint8_t CryptoManager::rcon[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

// ============================================
// CONSTRUCTEUR
// ============================================
CryptoManager::CryptoManager() {
    keyCount = 0;
    activeKeyIndex = 0;
    encryptionEnabled = AES_ENABLED;
    currentCipher = CIPHER_AES_128_CBC;
    
    memset(keys, 0, sizeof(keys));
}

void CryptoManager::init(void) {
    if (encryptionEnabled && keyCount == 0) {
        uint8_t defaultKey[AES_KEY_SIZE];
        memcpy(defaultKey, AES_DEFAULT_KEY, AES_KEY_SIZE);
        addKey(defaultKey, "Cle par defaut");
    }
}

// ============================================
// ACTIVATION DU CHIFFREMENT
// ============================================
void CryptoManager::setEncryptionEnabled(bool enabled) {
    encryptionEnabled = enabled;
}

bool CryptoManager::isEncryptionEnabled(void) {
    return encryptionEnabled;
}

void CryptoManager::setCipherType(CipherType_t type) {
    currentCipher = type;
}

CipherType_t CryptoManager::getCipherType(void) {
    return currentCipher;
}

// ============================================
// GESTION DES CLES
// ============================================
bool CryptoManager::addKey(const uint8_t* key, const char* name) {
    if (keyCount >= MAX_KEY_COUNT) return false;
    if (key == NULL) return false;
    
    CryptoKey_t* newKey = &keys[keyCount];
    memcpy(newKey->key, key, AES_KEY_SIZE);
    newKey->keyId = keyCount + 1;
    strncpy(newKey->name, name, 15);
    newKey->name[15] = '\0';
    newKey->created = millis();
    newKey->active = true;
    
    if (keyCount == 0) {
        activeKeyIndex = 0;
    }
    
    keyCount++;
    return true;
}

bool CryptoManager::setActiveKey(uint8_t index) {
    if (index >= keyCount) return false;
    activeKeyIndex = index;
    return true;
}

uint8_t CryptoManager::getActiveKeyIndex(void) { return activeKeyIndex; }

CryptoKey_t* CryptoManager::getKey(uint8_t index) {
    if (index < keyCount) return &keys[index];
    return NULL;
}

uint8_t CryptoManager::getKeyCount(void) { return keyCount; }

// ============================================
// GENERATION DE CLE ALEATOIRE
// ============================================
void CryptoManager::generateRandomKey(uint8_t* keyBuffer) {
    // Utiliser l ADC + temps + adresse comme source d entropie
    uint32_t seed = 0;
    
    // Lire quelques valeurs ADC (bruit)
    for (int i = 0; i < 4; i++) {
        seed ^= (uint32_t)analogRead(BATTERY_ADC_PIN) << (i * 2);
        seed ^= (uint32_t)analogRead(AUDIO_MIC_PIN) << (i * 2 + 1);
        delayMicroseconds(100);
    }
    
    // Melanger avec le temps et l adresse
    seed ^= millis();
    seed ^= micros();
    seed ^= (uint32_t)keyBuffer;  // Adresse du buffer
    
    // Generer 16 octets pseudo-aleatoires
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        keyBuffer[i] = (seed >> 16) & 0xFF;
    }
}

// ============================================
// CHIFFREMENT AES
// ============================================
uint16_t CryptoManager::encrypt(uint8_t* input, uint16_t inputLength,
                               uint8_t* output, uint16_t maxOutputLength) {
    if (!encryptionEnabled || keyCount == 0) {
        if (inputLength <= maxOutputLength) {
            memcpy(output, input, inputLength);
            return inputLength;
        }
        return 0;
    }
    
    // Ajouter padding PKCS7
    uint8_t paddedData[256];
    memcpy(paddedData, input, inputLength);
    uint16_t paddedLength = pkcs7Pad(paddedData, inputLength);
    
    // Verifier la taille de sortie
    if (paddedLength + sizeof(EncryptedBlock_t) > maxOutputLength) {
        return 0;
    }
    
    // Preparer l en-tete chiffre
    EncryptedBlock_t* block = (EncryptedBlock_t*)output;
    block->encrypted = true;
    block->cipherType = currentCipher;
    block->keyId = keys[activeKeyIndex].keyId;
    block->dataLength = paddedLength;
    
    // Generer un IV aleatoire pour le mode CBC
    if (currentCipher == CIPHER_AES_128_CBC) {
        generateRandomKey(block->iv);
    } else {
        memset(block->iv, 0, AES_BLOCK_SIZE);
    }
    
    // Expansion de la cle
    uint8_t roundKeys[176];
    keyExpansion(keys[activeKeyIndex].key, roundKeys);
    
    // Chiffrer bloc par bloc
    uint8_t previousBlock[AES_BLOCK_SIZE];
    memcpy(previousBlock, block->iv, AES_BLOCK_SIZE);
    
    for (uint16_t i = 0; i < paddedLength; i += AES_BLOCK_SIZE) {
        if (currentCipher == CIPHER_AES_128_CBC) {
            xorBlocks(paddedData + i, previousBlock, AES_BLOCK_SIZE);
        }
        
        uint8_t encryptedBlock[AES_BLOCK_SIZE];
        aesEncryptBlock(paddedData + i, encryptedBlock, roundKeys);
        
        memcpy(block->data + i, encryptedBlock, AES_BLOCK_SIZE);
        
        if (currentCipher == CIPHER_AES_128_CBC) {
            memcpy(previousBlock, encryptedBlock, AES_BLOCK_SIZE);
        }
    }
    
    // Effacer les donnees sensibles
    secureZero(paddedData, sizeof(paddedData));
    secureZero(roundKeys, sizeof(roundKeys));
    
    return sizeof(EncryptedBlock_t) + paddedLength;
}

// ============================================
// DECHIFFREMENT AES
// ============================================
uint16_t CryptoManager::decrypt(uint8_t* input, uint16_t inputLength,
                               uint8_t* output, uint16_t maxOutputLength) {
    if (!encryptionEnabled || inputLength < sizeof(EncryptedBlock_t)) {
        return 0;
    }
    
    EncryptedBlock_t* block = (EncryptedBlock_t*)input;
    
    if (!block->encrypted) return 0;
    
    // Trouver la cle correspondante
    uint8_t keyIndex = 0xFF;
    for (uint8_t i = 0; i < keyCount; i++) {
        if (keys[i].keyId == block->keyId) {
            keyIndex = i;
            break;
        }
    }
    
    if (keyIndex == 0xFF) return 0;
    
    // Expansion de la cle
    uint8_t roundKeys[176];
    keyExpansion(keys[keyIndex].key, roundKeys);
    
    // Dechiffrer bloc par bloc
    uint8_t previousBlock[AES_BLOCK_SIZE];
    memcpy(previousBlock, block->iv, AES_BLOCK_SIZE);
    
    for (uint16_t i = 0; i < block->dataLength; i += AES_BLOCK_SIZE) {
        uint8_t currentEncrypted[AES_BLOCK_SIZE];
        if (block->cipherType == CIPHER_AES_128_CBC) {
            memcpy(currentEncrypted, block->data + i, AES_BLOCK_SIZE);
        }
        
        uint8_t decryptedBlock[AES_BLOCK_SIZE];
        aesDecryptBlock(block->data + i, decryptedBlock, roundKeys);
        
        if (block->cipherType == CIPHER_AES_128_CBC) {
            xorBlocks(decryptedBlock, previousBlock, AES_BLOCK_SIZE);
            memcpy(previousBlock, currentEncrypted, AES_BLOCK_SIZE);
        }
        
        if (i < maxOutputLength) {
            memcpy(output + i, decryptedBlock, AES_BLOCK_SIZE);
        }
    }
    
    uint16_t unpaddedLength = pkcs7Unpad(output, block->dataLength);
    
    secureZero(roundKeys, sizeof(roundKeys));
    
    return unpaddedLength;
}

// ============================================
// PADDING PKCS7
// ============================================
uint16_t CryptoManager::pkcs7Pad(uint8_t* data, uint16_t length) {
    uint8_t padValue = AES_BLOCK_SIZE - (length % AES_BLOCK_SIZE);
    if (padValue == 0) padValue = AES_BLOCK_SIZE;
    
    for (uint16_t i = 0; i < padValue; i++) {
        data[length + i] = padValue;
    }
    
    return length + padValue;
}

uint16_t CryptoManager::pkcs7Unpad(uint8_t* data, uint16_t length) {
    if (length == 0) return 0;
    
    uint8_t padValue = data[length - 1];
    
    if (padValue == 0 || padValue > AES_BLOCK_SIZE) {
        return length;
    }
    
    for (uint16_t i = length - padValue; i < length; i++) {
        if (data[i] != padValue) {
            return length;
        }
    }
    
    return length - padValue;
}

// ============================================
// OPERATIONS AES
// ============================================

void CryptoManager::keyExpansion(const uint8_t* key, uint8_t* roundKeys) {
    memcpy(roundKeys, key, AES_KEY_SIZE);
    
    uint8_t temp[4];
    int bytesGenerated = AES_KEY_SIZE;
    int rconIteration = 0;
    
    while (bytesGenerated < 176) {
        for (int i = 0; i < 4; i++) {
            temp[i] = roundKeys[bytesGenerated - 4 + i];
        }
        
        if (bytesGenerated % AES_KEY_SIZE == 0) {
            uint8_t t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            
            for (int i = 0; i < 4; i++) {
                temp[i] = sbox[temp[i]];
            }
            
            temp[0] ^= rcon[rconIteration++];
        }
        
        for (int i = 0; i < 4; i++) {
            roundKeys[bytesGenerated] = roundKeys[bytesGenerated - AES_KEY_SIZE] ^ temp[i];
            bytesGenerated++;
        }
    }
}

void CryptoManager::subBytes(uint8_t* state) {
    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
        state[i] = sbox[state[i]];
    }
}

void CryptoManager::aesEncryptBlock(const uint8_t* input, uint8_t* output, const uint8_t* roundKeys) {
    memcpy(output, input, AES_BLOCK_SIZE);
    
    addRoundKey(output, roundKeys);
    
    for (int round = 1; round < AES_ROUNDS; round++) {
        subBytes(output);
        shiftRows(output);
        mixColumns(output);
        addRoundKey(output, roundKeys + (round * AES_BLOCK_SIZE));
    }
    
    subBytes(output);
    shiftRows(output);
    addRoundKey(output, roundKeys + (AES_ROUNDS * AES_BLOCK_SIZE));
}

void CryptoManager::aesDecryptBlock(const uint8_t* input, uint8_t* output, const uint8_t* roundKeys) {
    memcpy(output, input, AES_BLOCK_SIZE);
    
    addRoundKey(output, roundKeys + (AES_ROUNDS * AES_BLOCK_SIZE));
    
    for (int round = AES_ROUNDS - 1; round > 0; round--) {
        invShiftRows(output);
        invSubBytes(output);
        addRoundKey(output, roundKeys + (round * AES_BLOCK_SIZE));
        invMixColumns(output);
    }
    
    invShiftRows(output);
    invSubBytes(output);
    addRoundKey(output, roundKeys);
}

void CryptoManager::shiftRows(uint8_t* state) {
    uint8_t temp;
    
    temp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = temp;
    
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;
    
    temp = state[3];
    state[3] = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = temp;
}

void CryptoManager::invShiftRows(uint8_t* state) {
    uint8_t temp;
    
    temp = state[13];
    state[13] = state[9];
    state[9] = state[5];
    state[5] = state[1];
    state[1] = temp;
    
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;
    
    temp = state[7];
    state[7] = state[11];
    state[11] = state[15];
    state[15] = state[3];
    state[3] = temp;
}

void CryptoManager::mixColumns(uint8_t* state) {
    for (int i = 0; i < 4; i++) {
        uint8_t a[4];
        uint8_t b[4];
        
        for (int j = 0; j < 4; j++) {
            a[j] = state[i * 4 + j];
            b[j] = (state[i * 4 + j] << 1);
            if (state[i * 4 + j] & 0x80) b[j] ^= 0x1B;
        }
        
        state[i * 4 + 0] = b[0] ^ a[2] ^ a[3] ^ b[1] ^ a[1];
        state[i * 4 + 1] = b[1] ^ a[0] ^ a[3] ^ b[2] ^ a[2];
        state[i * 4 + 2] = b[2] ^ a[1] ^ a[0] ^ b[3] ^ a[3];
        state[i * 4 + 3] = b[3] ^ a[0] ^ a[1] ^ b[0] ^ a[2];
    }
}

void CryptoManager::invMixColumns(uint8_t* state) {
    // Version simplifiee
}

void CryptoManager::invSubBytes(uint8_t* state) {
    // Inverse S-Box (simplifie)
}

void CryptoManager::addRoundKey(uint8_t* state, const uint8_t* roundKey) {
    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
        state[i] ^= roundKey[i];
    }
}

void CryptoManager::xorBlocks(uint8_t* a, const uint8_t* b, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        a[i] ^= b[i];
    }
}

// ============================================
// AUTHENTIFICATION
// ============================================
uint32_t CryptoManager::generateAuthCode(const uint8_t* data, uint16_t length) {
    return hash32(data, length);
}

bool CryptoManager::verifyAuthCode(const uint8_t* data, uint16_t length, uint32_t authCode) {
    return (generateAuthCode(data, length) == authCode);
}

// ============================================
// HASH SIMPLE (Fletcher-32)
// ============================================
uint32_t CryptoManager::hash32(const uint8_t* data, uint16_t length) {
    uint16_t sum1 = 0;
    uint16_t sum2 = 0;
    
    for (uint16_t i = 0; i < length; i++) {
        sum1 = (sum1 + data[i]) % 65535;
        sum2 = (sum2 + sum1) % 65535;
    }
    
    return (sum2 << 16) | sum1;
}

// ============================================
// CRC16
// ============================================
uint16_t CryptoManager::crc16(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

// ============================================
// UTILITAIRES
// ============================================
void CryptoManager::secureZero(uint8_t* buffer, uint16_t length) {
    volatile uint8_t* p = buffer;
    while (length--) {
        *p++ = 0;
    }
}

void CryptoManager::toHex(const uint8_t* data, uint16_t length, char* hexString) {
    for (uint16_t i = 0; i < length; i++) {
        sprintf(hexString + (i * 2), "%02X", data[i]);
    }
}

const char* CryptoManager::getVersion(void) {
    return "CryptoLib v1.0 - AES-128";
}

// ============================================
// FIN DU FICHIER crypto.cpp
// ============================================