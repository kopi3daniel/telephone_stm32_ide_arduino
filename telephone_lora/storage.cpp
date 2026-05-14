/**
 * ---------------------------------------------------------------------------
 * storage.cpp - Implementation du stockage Flash
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * Utilise l EEPROM emulee du core STM32 Arduino pour la persistance.
 */

#include "storage.h"
#include <string.h>
#include <EEPROM.h>

// ============================================
// ADRESSES EEPROM (emulee dans la Flash)
// ============================================
#define EE_MAGIC_CONTACTS       0
#define EE_MAGIC_MESSAGES       4
#define EE_MAGIC_CALLLOG        8
#define EE_MAGIC_SETTINGS       12
#define EE_MAGIC_IDENTITY       16

#define EE_CONTACTS_START       32
#define EE_MESSAGES_START       (EE_CONTACTS_START + STORAGE_MAX_CONTACTS * sizeof(StoredContact_t) + 4)
#define EE_CALLLOG_START        (EE_MESSAGES_START + STORAGE_MAX_MESSAGES * sizeof(StoredMessage_t) + 4)
#define EE_SETTINGS_START       (EE_CALLLOG_START + STORAGE_MAX_CALL_LOG * sizeof(StoredCallLog_t) + 4)
#define EE_IDENTITY_START       (EE_SETTINGS_START + sizeof(StoredSettings_t) + 4)

// ============================================
// MAGIC NUMBERS
// ============================================
#define MAGIC_CONTACTS      0xCAFE0001
#define MAGIC_MESSAGES      0xCAFE0002
#define MAGIC_CALLLOG       0xCAFE0003
#define MAGIC_SETTINGS      0xCAFE0004
#define MAGIC_IDENTITY      0xCAFE0005

// ============================================
// CONSTRUCTEUR
// ============================================
Storage::Storage() {
    initialized = false;
    dataChanged = false;
    lastSave = 0;
    contactCount = 0;
    messageCount = 0;
    callLogCount = 0;
    
    memset(contacts, 0, sizeof(contacts));
    memset(messages, 0, sizeof(messages));
    memset(callLog, 0, sizeof(callLog));
    memset(&settings, 0, sizeof(settings));
    memset(&identity, 0, sizeof(identity));
}

void Storage::init(void) {
    // Configurer l identite par defaut
    identity.phoneId = PHONE_ID;
    strncpy(identity.phoneNumber, PHONE_NUMBER, 15);
    strncpy(identity.phoneName, "LoRaPhone", 31);
    identity.serialNumber = 0;
    identity.hardwareVersion = 1;
    identity.softwareVersion = 1;
    
    // Charger toutes les donnees
    loadAll();
    
    initialized = true;
}

// ============================================
// SAUVEGARDE / CHARGEMENT GLOBAL
// ============================================
void Storage::saveAll(void) {
    if (!initialized) return;
    
    saveContacts();
    saveMessages();
    saveCallLog();
    saveSettings();
    saveIdentity();
    
    dataChanged = false;
    lastSave = millis();
}

void Storage::loadAll(void) {
    if (!loadIdentity()) {
        saveIdentity();
    }
    
    loadContacts();
    loadMessages();
    loadCallLog();
    loadSettings();
}

void Storage::factoryReset(void) {
    clearContacts();
    clearMessages();
    clearCallLog();
    resetSettings();
    saveAll();
    dataChanged = true;
}

// ============================================
// FONCTIONS EEPROM UTILITAIRES
// ============================================
static void eeWriteU32(int addr, uint32_t value) {
    EEPROM.write(addr, value & 0xFF);
    EEPROM.write(addr + 1, (value >> 8) & 0xFF);
    EEPROM.write(addr + 2, (value >> 16) & 0xFF);
    EEPROM.write(addr + 3, (value >> 24) & 0xFF);
}

static uint32_t eeReadU32(int addr) {
    uint32_t val = 0;
    val |= (uint32_t)EEPROM.read(addr);
    val |= (uint32_t)EEPROM.read(addr + 1) << 8;
    val |= (uint32_t)EEPROM.read(addr + 2) << 16;
    val |= (uint32_t)EEPROM.read(addr + 3) << 24;
    return val;
}

static void eeWriteBuf(int addr, uint8_t* data, int len) {
    for (int i = 0; i < len; i++) {
        EEPROM.write(addr + i, data[i]);
    }
}

static void eeReadBuf(int addr, uint8_t* data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] = EEPROM.read(addr + i);
    }
}

static void eeWriteU16(int addr, uint16_t value) {
    EEPROM.write(addr, value & 0xFF);
    EEPROM.write(addr + 1, (value >> 8) & 0xFF);
}

static uint16_t eeReadU16(int addr) {
    uint16_t val = 0;
    val |= (uint16_t)EEPROM.read(addr);
    val |= (uint16_t)EEPROM.read(addr + 1) << 8;
    return val;
}

uint32_t Storage::calculateChecksum(uint8_t* data, uint32_t length) {
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum;
}

// ============================================
// CONTACTS
// ============================================
bool Storage::loadContacts(void) {
    uint32_t magic = eeReadU32(EE_MAGIC_CONTACTS);
    if (magic != MAGIC_CONTACTS) {
        contactCount = 0;
        return false;
    }
    
    contactCount = eeReadU16(EE_CONTACTS_START);
    
    if (contactCount > STORAGE_MAX_CONTACTS) {
        contactCount = 0;
        return false;
    }
    
    eeReadBuf(EE_CONTACTS_START + 2, (uint8_t*)contacts, contactCount * sizeof(StoredContact_t));
    
    return true;
}

bool Storage::saveContacts(void) {
    eeWriteU32(EE_MAGIC_CONTACTS, MAGIC_CONTACTS);
    eeWriteU16(EE_CONTACTS_START, contactCount);
    eeWriteBuf(EE_CONTACTS_START + 2, (uint8_t*)contacts, contactCount * sizeof(StoredContact_t));
    return true;
}

uint16_t Storage::getContactCount(void) { return contactCount; }

bool Storage::getContact(uint16_t index, char* name, char* number, bool* favorite) {
    if (index >= contactCount) return false;
    
    strncpy(name, contacts[index].name, 31);
    name[31] = '\0';
    strncpy(number, contacts[index].number, 15);
    number[15] = '\0';
    *favorite = (contacts[index].favorite == 1);
    
    return true;
}

bool Storage::addContact(const char* name, const char* number) {
    if (contactCount >= STORAGE_MAX_CONTACTS || name == NULL || number == NULL) return false;
    
    strncpy(contacts[contactCount].name, name, 31);
    contacts[contactCount].name[31] = '\0';
    strncpy(contacts[contactCount].number, number, 15);
    contacts[contactCount].number[15] = '\0';
    contacts[contactCount].favorite = 0;
    
    contactCount++;
    dataChanged = true;
    return true;
}

bool Storage::updateContact(uint16_t index, const char* name, const char* number) {
    if (index >= contactCount) return false;
    
    if (name) {
        strncpy(contacts[index].name, name, 31);
        contacts[index].name[31] = '\0';
    }
    if (number) {
        strncpy(contacts[index].number, number, 15);
        contacts[index].number[15] = '\0';
    }
    
    dataChanged = true;
    return true;
}

bool Storage::deleteContact(uint16_t index) {
    if (index >= contactCount) return false;
    
    for (uint16_t i = index; i < contactCount - 1; i++) {
        contacts[i] = contacts[i + 1];
    }
    contactCount--;
    dataChanged = true;
    return true;
}

bool Storage::setFavorite(uint16_t index, bool favorite) {
    if (index >= contactCount) return false;
    contacts[index].favorite = favorite ? 1 : 0;
    dataChanged = true;
    return true;
}

void Storage::clearContacts(void) {
    memset(contacts, 0, sizeof(contacts));
    contactCount = 0;
    dataChanged = true;
}

// ============================================
// MESSAGES
// ============================================
bool Storage::loadMessages(void) {
    uint32_t magic = eeReadU32(EE_MAGIC_MESSAGES);
    if (magic != MAGIC_MESSAGES) {
        messageCount = 0;
        return false;
    }
    
    messageCount = eeReadU16(EE_MESSAGES_START);
    
    if (messageCount > STORAGE_MAX_MESSAGES) {
        messageCount = 0;
        return false;
    }
    
    eeReadBuf(EE_MESSAGES_START + 2, (uint8_t*)messages, messageCount * sizeof(StoredMessage_t));
    
    return true;
}

bool Storage::saveMessages(void) {
    eeWriteU32(EE_MAGIC_MESSAGES, MAGIC_MESSAGES);
    eeWriteU16(EE_MESSAGES_START, messageCount);
    eeWriteBuf(EE_MESSAGES_START + 2, (uint8_t*)messages, messageCount * sizeof(StoredMessage_t));
    return true;
}

uint16_t Storage::getMessageCount(void) { return messageCount; }

bool Storage::addMessage(const char* text, const char* sender, bool outgoing) {
    if (messageCount >= STORAGE_MAX_MESSAGES) return false;
    
    strncpy(messages[messageCount].text, text, 159);
    messages[messageCount].text[159] = '\0';
    strncpy(messages[messageCount].sender, sender, 15);
    messages[messageCount].sender[15] = '\0';
    messages[messageCount].timestamp = millis();
    messages[messageCount].read = false;
    messages[messageCount].outgoing = outgoing ? 1 : 0;
    
    messageCount++;
    dataChanged = true;
    return true;
}

bool Storage::markMessageRead(uint16_t index) {
    if (index >= messageCount) return false;
    messages[index].read = 1;
    dataChanged = true;
    return true;
}

bool Storage::deleteMessage(uint16_t index) {
    if (index >= messageCount) return false;
    for (uint16_t i = index; i < messageCount - 1; i++) {
        messages[i] = messages[i + 1];
    }
    messageCount--;
    dataChanged = true;
    return true;
}

void Storage::clearMessages(void) {
    memset(messages, 0, sizeof(messages));
    messageCount = 0;
    dataChanged = true;
}

// ============================================
// JOURNAL D APPELS
// ============================================
bool Storage::loadCallLog(void) {
    uint32_t magic = eeReadU32(EE_MAGIC_CALLLOG);
    if (magic != MAGIC_CALLLOG) {
        callLogCount = 0;
        return false;
    }
    
    callLogCount = eeReadU16(EE_CALLLOG_START);
    
    if (callLogCount > STORAGE_MAX_CALL_LOG) {
        callLogCount = 0;
        return false;
    }
    
    eeReadBuf(EE_CALLLOG_START + 2, (uint8_t*)callLog, callLogCount * sizeof(StoredCallLog_t));
    
    return true;
}

bool Storage::saveCallLog(void) {
    eeWriteU32(EE_MAGIC_CALLLOG, MAGIC_CALLLOG);
    eeWriteU16(EE_CALLLOG_START, callLogCount);
    eeWriteBuf(EE_CALLLOG_START + 2, (uint8_t*)callLog, callLogCount * sizeof(StoredCallLog_t));
    return true;
}

uint16_t Storage::getCallLogCount(void) { return callLogCount; }

bool Storage::addCallLogEntry(const char* number, uint32_t duration, uint8_t type) {
    if (callLogCount >= STORAGE_MAX_CALL_LOG) {
        for (uint16_t i = 0; i < STORAGE_MAX_CALL_LOG - 1; i++) {
            callLog[i] = callLog[i + 1];
        }
        callLogCount = STORAGE_MAX_CALL_LOG - 1;
    }
    
    strncpy(callLog[callLogCount].number, number, 15);
    callLog[callLogCount].number[15] = '\0';
    callLog[callLogCount].duration = duration;
    callLog[callLogCount].timestamp = millis();
    callLog[callLogCount].type = type;
    
    callLogCount++;
    dataChanged = true;
    return true;
}

void Storage::clearCallLog(void) {
    memset(callLog, 0, sizeof(callLog));
    callLogCount = 0;
    dataChanged = true;
}

// ============================================
// PARAMETRES
// ============================================
bool Storage::loadSettings(void) {
    uint32_t magic = eeReadU32(EE_MAGIC_SETTINGS);
    if (magic != MAGIC_SETTINGS) {
        resetSettings();
        return false;
    }
    
    eeReadBuf(EE_SETTINGS_START, (uint8_t*)&settings, sizeof(StoredSettings_t));
    return true;
}

bool Storage::saveSettings(void) {
    eeWriteU32(EE_MAGIC_SETTINGS, MAGIC_SETTINGS);
    eeWriteBuf(EE_SETTINGS_START, (uint8_t*)&settings, sizeof(StoredSettings_t));
    return true;
}

StoredSettings_t* Storage::getSettings(void) { return &settings; }

void Storage::setSettings(StoredSettings_t* newSettings) {
    if (newSettings) {
        memcpy(&settings, newSettings, sizeof(StoredSettings_t));
        dataChanged = true;
    }
}

void Storage::resetSettings(void) {
    memset(&settings, 0, sizeof(settings));
    settings.ringVolume = 7;
    settings.ringTone = 0;
    settings.vibrateEnabled = 1;
    settings.silentMode = 0;
    settings.brightness = 200;
    settings.screenTimeout = 30;
    settings.frequencyIndex = 0;
    settings.txPower = 17;
    settings.encryptionEnabled = 0;
    settings.lockEnabled = 0;
    strcpy(settings.pinCode, "0000");
    settings.powerSaving = 1;
    settings.sleepTimeout = 5;
    dataChanged = true;
}

// ============================================
// IDENTITE
// ============================================
bool Storage::loadIdentity(void) {
    uint32_t magic = eeReadU32(EE_MAGIC_IDENTITY);
    if (magic != MAGIC_IDENTITY) {
        return false;
    }
    
    eeReadBuf(EE_IDENTITY_START, (uint8_t*)&identity, sizeof(StoredIdentity_t));
    return true;
}

bool Storage::saveIdentity(void) {
    eeWriteU32(EE_MAGIC_IDENTITY, MAGIC_IDENTITY);
    eeWriteBuf(EE_IDENTITY_START, (uint8_t*)&identity, sizeof(StoredIdentity_t));
    return true;
}

StoredIdentity_t* Storage::getIdentity(void) { return &identity; }

// ============================================
// ETAT
// ============================================
bool Storage::isDataChanged(void) { return dataChanged; }
uint32_t Storage::getLastSaveTime(void) { return lastSave; }

// ============================================
// FIN DU FICHIER storage.cpp
// ============================================