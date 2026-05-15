/**
 * Ce fichier declare la classe CryptoManager qui gere :
 * - Le chiffrement/dechiffrement AES-128 des communications
 * - La gestion des cles partagees
 * - L authentification par ID unique
 * - La verification d integrite (hash simple)
 * - La generation de nombres aleatoires
 * - La protection des donnees sensibles
 */

#ifndef CRYPTO_H
#define CRYPTO_H

#include <Arduino.h>
#include "config.h"

// ============================================
// CONSTANTES DE CHIFFREMENT
// ============================================
#define AES_BLOCK_SIZE      16      // Taille d un bloc AES (128 bits)
#define AES_KEY_SIZE        16      // Taille d une cle AES-128
#define AES_ROUNDS          10      // Nombre de rounds pour AES-128
#define MAX_KEY_COUNT       10      // Nombre max de cles stockees

// ============================================
// TYPES DE CHIFFREMENT
// ============================================
typedef enum {
    CIPHER_NONE,            // Pas de chiffrement
    CIPHER_AES_128_ECB,     // AES-128 mode ECB (simple)
    CIPHER_AES_128_CBC      // AES-128 mode CBC (recommande)
} CipherType_t;

// ============================================
// STRUCTURE D UNE CLE
// ============================================
typedef struct {
    uint8_t key[AES_KEY_SIZE];      // La cle elle-meme (16 octets)
    uint16_t keyId;                  // Identifiant de la cle
    char     name[16];               // Nom descriptif
    uint32_t created;                // Date de creation
    bool     active;                 // Cle active ?
} CryptoKey_t;

// ============================================
// STRUCTURE POUR UN BLOC CHIFFRE
// ============================================
typedef struct {
    uint8_t  encrypted;              // true = chiffre, false = clair
    uint8_t  cipherType;            // Type de chiffrement utilise
    uint16_t keyId;                  // Quelle cle a ete utilisee
    uint8_t  iv[AES_BLOCK_SIZE];    // Vecteur d initialisation (CBC)
    uint8_t  data[256];             // Donnees chiffrees
    uint16_t dataLength;             // Longueur des donnees
} EncryptedBlock_t;

// ============================================
// CLASSE PRINCIPALE
// ============================================
class CryptoManager {
    
private:
    // ============================================
    // CLES STOCKEES
    // ============================================
    CryptoKey_t keys[MAX_KEY_COUNT];
    uint8_t keyCount;
    uint8_t activeKeyIndex;         // Index de la cle active
    
    // ============================================
    // ETAT
    // ============================================
    bool encryptionEnabled;         // Chiffrement active globalement ?
    CipherType_t currentCipher;     // Type de chiffrement actuel
    
    // ============================================
    // TABLES DE L AES
    // ============================================
    
    // S-Box (table de substitution)
    static const uint8_t sbox[256];
    
    // Inverse S-Box (simplifiee - non utilisee ici)
    static const uint8_t rsbox[256];
    
    // Table de multiplication par 2 dans GF(2^8) (simplifiee)
    static const uint8_t mul2[256];
    
    // Table de multiplication par 3 dans GF(2^8) (simplifiee)
    static const uint8_t mul3[256];
    
    // Constante de round (Rcon)
    static const uint8_t rcon[10];
    
    // ============================================
    // FONCTIONS AES INTERNES
    // ============================================
    
    // Etend la cle (Key Expansion)
    void keyExpansion(const uint8_t* key, uint8_t* roundKeys);
    
    // SubBytes : substitution de chaque octet via la S-Box
    void subBytes(uint8_t* state);
    
    // Inverse SubBytes
    void invSubBytes(uint8_t* state);
    
    // ShiftRows : decalage des lignes
    void shiftRows(uint8_t* state);
    
    // Inverse ShiftRows
    void invShiftRows(uint8_t* state);
    
    // MixColumns : melange des colonnes
    void mixColumns(uint8_t* state);
    
    // Inverse MixColumns
    void invMixColumns(uint8_t* state);
    
    // AddRoundKey : XOR avec la cle de round
    void addRoundKey(uint8_t* state, const uint8_t* roundKey);
    
    // Chiffre un bloc de 16 octets
    void aesEncryptBlock(const uint8_t* input, uint8_t* output, const uint8_t* roundKeys);
    
    // Dechiffre un bloc de 16 octets
    void aesDecryptBlock(const uint8_t* input, uint8_t* output, const uint8_t* roundKeys);
    
    // Padding PKCS7 pour aligner les donnees sur 16 octets
    uint16_t pkcs7Pad(uint8_t* data, uint16_t length);
    
    // Supprime le padding PKCS7
    uint16_t pkcs7Unpad(uint8_t* data, uint16_t length);
    
    // XOR deux buffers
    void xorBlocks(uint8_t* a, const uint8_t* b, uint8_t length);
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    CryptoManager();
    void init(void);
    
    // ============================================
    // ACTIVATION DU CHIFFREMENT
    // ============================================
    
    // Active/desactive le chiffrement global
    void setEncryptionEnabled(bool enabled);
    
    // Verifie si le chiffrement est active
    bool isEncryptionEnabled(void);
    
    // Definit le type de chiffrement
    void setCipherType(CipherType_t type);
    
    // Retourne le type de chiffrement actuel
    CipherType_t getCipherType(void);
    
    // ============================================
    // GESTION DES CLES
    // ============================================
    
    // Ajoute une nouvelle cle
    bool addKey(const uint8_t* key, const char* name);
    
    // Supprime une cle par index
    bool removeKey(uint8_t index);
    
    // Active une cle specifique
    bool setActiveKey(uint8_t index);
    
    // Retourne l index de la cle active
    uint8_t getActiveKeyIndex(void);
    
    // Retourne une cle par index
    CryptoKey_t* getKey(uint8_t index);
    
    // Retourne le nombre de cles stockees
    uint8_t getKeyCount(void);
    
    // Genere une cle aleatoire
    void generateRandomKey(uint8_t* keyBuffer);
    
    // Importe une cle depuis une chaine hexadecimale
    bool importKeyFromHex(const char* hexString, const char* name);
    
    // Exporte la cle active en chaine hexadecimale
    void exportKeyToHex(char* hexString);
    
    // ============================================
    // CHIFFREMENT / DECHIFFREMENT
    // ============================================
    
    // Chiffre un bloc de donnees
    uint16_t encrypt(uint8_t* input, uint16_t inputLength,
                    uint8_t* output, uint16_t maxOutputLength);
    
    // Dechiffre un bloc de donnees
    uint16_t decrypt(uint8_t* input, uint16_t inputLength,
                    uint8_t* output, uint16_t maxOutputLength);
    
    // Chiffre un paquet LoRa complet
    bool encryptPacket(uint8_t* packet, uint16_t* packetLength);
    
    // Dechiffre un paquet LoRa complet
    bool decryptPacket(uint8_t* packet, uint16_t* packetLength);
    
    // ============================================
    // AUTHENTIFICATION
    // ============================================
    
    // Verifie l identite d un expediteur
    bool verifySender(uint16_t senderId);
    
    // Genere un code d authentification simple (hash)
    uint32_t generateAuthCode(const uint8_t* data, uint16_t length);
    
    // Verifie un code d authentification
    bool verifyAuthCode(const uint8_t* data, uint16_t length, uint32_t authCode);
    
    // ============================================
    // FONCTIONS DE HASH SIMPLE
    // ============================================
    
    // Hash simple 32 bits (Fletcher-32)
    uint32_t hash32(const uint8_t* data, uint16_t length);
    
    // CRC16 pour l integrite des paquets
    uint16_t crc16(const uint8_t* data, uint16_t length);
    
    // ============================================
    // UTILITAIRES
    // ============================================
    
    // Efface securise d un buffer
    void secureZero(uint8_t* buffer, uint16_t length);
    
    // Compare deux buffers en temps constant (anti-timing attack)
    bool constantTimeCompare(const uint8_t* a, const uint8_t* b, uint16_t length);
    
    // Convertit un buffer en chaine hexadecimale
    void toHex(const uint8_t* data, uint16_t length, char* hexString);
    
    // Convertit une chaine hexadecimale en buffer
    bool fromHex(const char* hexString, uint8_t* data, uint16_t* length);
    
    // Retourne la version de la bibliotheque crypto
    const char* getVersion(void);
};

#endif // CRYPTO_H