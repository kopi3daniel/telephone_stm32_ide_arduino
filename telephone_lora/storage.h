/**

 * Ce module gere la sauvegarde et le chargement des donnees
 * dans la memoire Flash du STM32F103C8T6 (64KB).
 * 
 * Donnees stockees :
 * - Contacts (max 100)
 * - Messages SMS (max 100)
 * - Journal d appels (max 50)
 * - Parametres
 * - Identite du telephone
 * 
 * Utilise l EEPROM emulee du core STM32 Arduino.
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include "config.h"

// ============================================
// TAILLES MAXIMALES
// ============================================
#define STORAGE_MAX_CONTACTS    100
#define STORAGE_MAX_MESSAGES    100
#define STORAGE_MAX_CALL_LOG    50
#define STORAGE_SETTINGS_SIZE   128

// ============================================
// STRUCTURES DE DONNEES STOCKEES
// ============================================

// Contact (52 octets)
typedef struct {
    char name[32];
    char number[16];
    uint8_t favorite;
    uint8_t reserved[3];
} StoredContact_t;

// Message SMS (184 octets)
typedef struct {
    char text[160];
    char sender[16];
    uint32_t timestamp;
    uint8_t read;
    uint8_t outgoing;
    uint8_t reserved[2];
} StoredMessage_t;

// Entree du journal d appels (24 octets)
typedef struct {
    char number[16];
    uint32_t duration;
    uint32_t timestamp;
    uint8_t type;
    uint8_t reserved[3];
} StoredCallLog_t;

// Parametres (128 octets)
typedef struct {
    uint8_t ringVolume;
    uint8_t ringTone;
    uint8_t vibrateEnabled;
    uint8_t silentMode;
    uint8_t brightness;
    uint8_t screenTimeout;
    uint8_t frequencyIndex;
    uint8_t txPower;
    uint8_t encryptionEnabled;
    uint8_t lockEnabled;
    char pinCode[5];
    uint8_t powerSaving;
    uint8_t sleepTimeout;
    uint8_t reserved[113];
} StoredSettings_t;

// Identite du telephone (64 octets)
typedef struct {
    uint16_t phoneId;
    char phoneNumber[16];
    char phoneName[32];
    uint32_t serialNumber;
    uint8_t hardwareVersion;
    uint8_t softwareVersion;
    uint8_t reserved[8];
} StoredIdentity_t;

// ============================================
// CLASSE STORAGE
// ============================================
class Storage {
    
private:
    // ============================================
    // ETAT
    // ============================================
    bool initialized;
    bool dataChanged;
    uint32_t lastSave;
    
    // ============================================
    // BUFFERS EN RAM
    // ============================================
    StoredContact_t contacts[STORAGE_MAX_CONTACTS];
    uint16_t contactCount;
    
    StoredMessage_t messages[STORAGE_MAX_MESSAGES];
    uint16_t messageCount;
    
    StoredCallLog_t callLog[STORAGE_MAX_CALL_LOG];
    uint16_t callLogCount;
    
    StoredSettings_t settings;
    StoredIdentity_t identity;
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    uint32_t calculateChecksum(uint8_t* data, uint32_t length);
    
public:
    // ============================================
    // CONSTRUCTEUR
    // ============================================
    Storage();
    
    // ============================================
    // INITIALISATION
    // ============================================
    void init(void);
    
    // ============================================
    // SAUVEGARDE / CHARGEMENT GLOBAL
    // ============================================
    
    void saveAll(void);
    void loadAll(void);
    void factoryReset(void);
    
    // ============================================
    // CONTACTS
    // ============================================
    bool loadContacts(void);
    bool saveContacts(void);
    uint16_t getContactCount(void);
    bool getContact(uint16_t index, char* name, char* number, bool* favorite);
    bool addContact(const char* name, const char* number);
    bool updateContact(uint16_t index, const char* name, const char* number);
    bool deleteContact(uint16_t index);
    bool setFavorite(uint16_t index, bool favorite);
    void clearContacts(void);
    
    // ============================================
    // MESSAGES
    // ============================================
    bool loadMessages(void);
    bool saveMessages(void);
    uint16_t getMessageCount(void);
    bool getMessage(uint16_t index, char* text, char* sender, uint32_t* timestamp, bool* read);
    bool addMessage(const char* text, const char* sender, bool outgoing);
    bool markMessageRead(uint16_t index);
    bool deleteMessage(uint16_t index);
    void clearMessages(void);
    
    // ============================================
    // JOURNAL D APPELS
    // ============================================
    bool loadCallLog(void);
    bool saveCallLog(void);
    uint16_t getCallLogCount(void);
    bool getCallLogEntry(uint16_t index, char* number, uint32_t* duration, uint32_t* timestamp, uint8_t* type);
    bool addCallLogEntry(const char* number, uint32_t duration, uint8_t type);
    void clearCallLog(void);
    
    // ============================================
    // PARAMETRES
    // ============================================
    bool loadSettings(void);
    bool saveSettings(void);
    StoredSettings_t* getSettings(void);
    void setSettings(StoredSettings_t* newSettings);
    void resetSettings(void);
    
    // ============================================
    // IDENTITE
    // ============================================
    bool loadIdentity(void);
    bool saveIdentity(void);
    StoredIdentity_t* getIdentity(void);
    
    // ============================================
    // ETAT
    // ============================================
    bool isDataChanged(void);
    uint32_t getLastSaveTime(void);
};

#endif // STORAGE_H