/**
 * ---------------------------------------------------------------------------
 * lora_driver.h - Driver pour le module LoRa RA-02 (SX1278)
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Ce fichier declare la classe LoRaDriver qui gere :
 * - L initialisation du module SX1278 via SPI
 * - L envoi de paquets radio
 * - La reception de paquets radio (avec interruption DIO0)
 * - La configuration des parametres (frequence, SF, BW, puissance)
 * - Les modes : SLEEP, STANDBY, TX, RX continu, RX cyclique
 * - La mesure RSSI/SNR
 * - La detection de canal occupe (CAD)
 * - Les deux profils : Voix (rapide) et Data (fiable)
 * 
 * Bibliotheque requise : RadioLib (par Jan Gromes)
 * Installation : Outils > Gerer les bibliotheques > rechercher "RadioLib"
 */

#ifndef LORA_DRIVER_H
#define LORA_DRIVER_H

#include <Arduino.h>
#include <SPI.h>
#include "config.h"


// #ifndef LORA_DRIVER_H
// #define LORA_DRIVER_H

// #include <Arduino.h>
// #include <SPI.h>
// #include "config.h"
// ============================================
// MODES DE FONCTIONNEMENT DU MODULE LORA
// ============================================
typedef enum {
    LORA_MODE_SLEEP,        // Module en sommeil (consommation minimale)
    LORA_MODE_STANDBY,      // Module reveille mais pas d ecoute
    LORA_MODE_TX,           // Transmission en cours
    LORA_MODE_RX_CONTINUOUS,// Reception continue (ecoute permanente)
    LORA_MODE_RX_CYCLIC,    // Reception cyclique (ecoute par intervalles)
    LORA_MODE_CAD           // Channel Activity Detection
} LoRaMode_t;

// ============================================
// PROFILS DE CONFIGURATION LORA
// ============================================
typedef enum {
    LORA_PROFILE_VOICE,     // Mode voix : SF7, BW250kHz (rapide, ~30ms latence)
    LORA_PROFILE_DATA       // Mode donnees : SF9, BW125kHz (fiable, longue portee)
} LoRaProfile_t;

// ============================================
// STRUCTURE POUR UN PAQUET LORA
// ============================================
typedef struct {
    uint8_t  syncByte;          // Octet de synchronisation (0xAA)
    uint8_t  packetType;        // Type de paquet (CALL_REQUEST, SMS_TEXT, etc.)
    uint16_t senderId;          // ID de l emetteur (mon telephone)
    uint16_t targetId;          // ID du destinataire (0xFFFF = broadcast)
    uint16_t sequenceNum;       // Numero de sequence (pour ordre et detection perte)
    uint8_t  payloadSize;       // Taille des donnees utiles (max 250 octets)
    uint8_t  payload[PKT_MAX_PAYLOAD]; // Donnees utiles
} LoRaPacket_t;

// ============================================
// STATISTIQUES DE TRANSMISSION
// ============================================
typedef struct {
    uint32_t packetsSent;       // Nombre de paquets envoyes
    uint32_t packetsReceived;   // Nombre de paquets recus
    uint32_t packetsLost;       // Paquets perdus (detectes par trou dans sequence)
    uint32_t crcErrors;         // Erreurs CRC
    int16_t  lastRSSI;         // Derniere valeur RSSI (dBm)
    int8_t   lastSNR;          // Dernier rapport signal/bruit (dB)
    uint32_t totalBytesSent;   // Total octets emis
    uint32_t totalBytesReceived;// Total octets recus
} LoRaStats_t;

// ============================================
// CLASSE PRINCIPALE : LORADRIVER
// ============================================
class LoRaDriver {
    
private:
    // ============================================
    // PINS SPI
    // ============================================
    // Les pins sont definies dans config.h :
    // LORA_NSS_PIN  (PA4)
    // LORA_SCK_PIN  (PA5)
    // LORA_MISO_PIN (PA6)
    // LORA_MOSI_PIN (PA7)
    // LORA_DIO0_PIN (PA0)
    // LORA_RST_PIN  (PA2)
    
    // ============================================
    // ETAT DU MODULE
    // ============================================
    LoRaMode_t currentMode;         // Mode actuel
    LoRaProfile_t currentProfile;   // Profil actif (voix ou data)
    bool packetAvailable;           // Un paquet est disponible en reception
    bool transmitting;              // Transmission en cours
    
    // ============================================
    // PARAMETRES DE CONFIGURATION
    // ============================================
    float frequency;            // Frequence actuelle (868.0, 915.0, 433.0)
    float bandwidth;            // Bande passante (125, 250, 500 kHz)
    uint8_t spreadingFactor;    // Facteur d etalement (6-12)
    uint8_t codingRate;         // Taux de codage (5-8)
    int8_t txPower;            // Puissance d emission (2-20 dBm)
    uint16_t preambleLength;    // Longueur du preambule (symboles)
    bool crcEnabled;           // CRC active ?
    
    // ============================================
    // BUFFERS DE RECEPTION
    // ============================================
    uint8_t rxBuffer[256];      // Buffer pour la reception
    uint8_t rxBufferIndex;      // Index dans le buffer
    uint16_t expectedSeqNum;    // Numero de sequence attendu
    
    // ============================================
    // STATISTIQUES
    // ============================================
    LoRaStats_t stats;
    
    // ============================================
    // MODE CYCLIQUE (economie d energie)
    // ============================================
    uint32_t cyclicOnTime;      // Temps d ecoute (ms)
    uint32_t cyclicOffTime;     // Temps de sommeil (ms)
    uint32_t lastCyclicWake;    // Dernier reveil cyclique
    
    // ============================================
    // METHODES PRIVEES - COMMUNICATION SPI
    // ============================================
    
    // Lit un registre du SX1278
    uint8_t readRegister(uint8_t addr);
    
    // Ecrit dans un registre du SX1278
    void writeRegister(uint8_t addr, uint8_t value);
    
    // Lit plusieurs registres consecutifs (burst read)
    void readBurst(uint8_t addr, uint8_t* buffer, uint8_t length);
    
    // Ecrit plusieurs registres consecutifs (burst write)
    void writeBurst(uint8_t addr, uint8_t* buffer, uint8_t length);
    
    // ============================================
    // METHODES PRIVEES - CONFIGURATION
    // ============================================
    
    // Configure les broches GPIO (NSS, RESET, DIO0)
    void initGPIO(void);
    
    // Reinitialise le module (hardware reset)
    void resetModule(void);
    
    // Configure le mode de fonctionnement
    void setMode(LoRaMode_t mode);
    
    // Configure la frequence
    void setFrequency(float freq);
    
    // Configure les parametres LoRa (SF, BW, CR)
    void setLoRaParameters(void);
    
    // Configure la puissance d emission
    void setTxPower(int8_t power);
    
    // Verifie si un paquet est en cours de reception
    bool isRxInProgress(void);
    
    // Vide le buffer FIFO de reception
    void clearRxFIFO(void);
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    LoRaDriver();
    
    // Initialise le module LoRa
    void init(void);
    
    // ============================================
    // CONFIGURATION DES PROFILS
    // ============================================
    
    // Configure le module pour le mode Voix ou Data
    void setProfile(LoRaProfile_t profile);
    
    // Retourne le profil actuel
    LoRaProfile_t getProfile(void);
    
    // ============================================
    // ENVOI DE PAQUETS
    // ============================================
    
    // Envoie un paquet structure
    bool sendPacket(LoRaPacket_t* packet);
    
    // Envoie des donnees brutes (utilise en interne par sendPacket)
    bool sendRaw(uint8_t* data, uint16_t length);
    
    // Envoie un paquet en broadcast (a tous)
    bool sendBroadcast(uint8_t* data, uint16_t length);
    
    // Verifie si une transmission est en cours
    bool isTransmitting(void);
    
    // ============================================
    // RECEPTION DE PAQUETS
    // ============================================
    
    // Passe en mode reception continue
    void startReceive(void);
    
    // Passe en mode reception cyclique (economie d energie)
    void startCyclicReceive(uint32_t onTime, uint32_t offTime);
    
    // Verifie si un paquet est disponible
    bool isPacketAvailable(void);
    
    // Lit le paquet recu (remplit la structure)
    bool receivePacket(LoRaPacket_t* packet);
    
    // Lit des donnees brutes recues
    uint16_t receiveRaw(uint8_t* buffer, uint16_t maxLen);
    
    // ============================================
    // GESTION DE L INTERRUPTION DIO0
    // ============================================
    
    // A appeler dans le handler d interruption
    void handleInterrupt(void);
    
    // ============================================
    // MESURES RADIO
    // ============================================
    
    // Mesure le RSSI (Received Signal Strength Indicator)
    int16_t getRSSI(void);
    
    // Mesure le SNR (Signal-to-Noise Ratio)
    int8_t getSNR(void);
    
    // Detecte si le canal est occupe (CAD)
    bool isChannelBusy(void);
    
    // ============================================
    // CONTROLE DU MODULE
    // ============================================
    
    // Met le module en sommeil
    void sleep(void);
    
    // Reveille le module
    void wakeup(void);
    
    // Eteint completement le module
    void powerDown(void);
    
    // Retourne le mode actuel
    LoRaMode_t getMode(void);
    
    // ============================================
    // STATISTIQUES
    // ============================================
    
    // Retourne les statistiques
    LoRaStats_t getStats(void);
    
    // Reinitialise les statistiques
    void resetStats(void);
    
    // ============================================
    // DIAGNOSTIC
    // ============================================
    
    // Verifie si le module repond (test de communication SPI)
    bool isAlive(void);
    
    // Retourne la version du silicium
    uint8_t getVersion(void);
    
    // Test de boucle locale (loopback sans antenne)
    bool selfTest(void);
};

#endif // LORA_DRIVER_H