/**
 * ---------------------------------------------------------------------------
 * sms_manager.h - Gestion des messages texte (SMS) pour Telephone LoRa
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Ce fichier declare la classe SMSManager qui gere :
 * - L envoi de SMS (max 160 caracteres)
 * - La reception de SMS
 * - Le stockage des conversations
 * - La fragmentation des messages longs
 * - Les accuses de reception
 * - Les notifications de nouveaux messages
 */

#ifndef SMS_MANAGER_H
#define SMS_MANAGER_H

#include <Arduino.h>
#include "config.h"

// ============================================
// ETATS D UN SMS
// ============================================
typedef enum {
    SMS_STATE_IDLE,             // Aucune operation en cours
    SMS_STATE_COMPOSING,        // Redaction du message
    SMS_STATE_SENDING,          // Envoi en cours
    SMS_STATE_SENT,             // Envoye (attente accuse)
    SMS_STATE_DELIVERED,        // Livre confirme
    SMS_STATE_FAILED,           // Echec d envoi
    SMS_STATE_RECEIVING,        // Reception en cours
    SMS_STATE_RECEIVED          // Message recu
} SMSState_t;

// ============================================
// STRUCTURE D UN MESSAGE
// ============================================
typedef struct {
    uint16_t messageId;         // Identifiant unique du message
    char sender[16];            // Numero de l expediteur
    char recipient[16];         // Numero du destinataire
    char text[SMS_MAX_LENGTH + 1]; // Texte du message (+1 pour '\0')
    uint32_t timestamp;         // Horodatage
    bool read;                  // Lu ou non lu
    bool outgoing;              // true = envoye, false = recu
    bool delivered;             // Accuse de reception recu ?
} Message_t;

// ============================================
// STRUCTURE D UNE CONVERSATION
// ============================================
typedef struct {
    char contact[16];           // Numero du contact
    char contactName[32];       // Nom du contact (si trouve)
    Message_t messages[SMS_MAX_PER_CONVERSATION]; // Messages
    uint8_t messageCount;       // Nombre de messages
    uint8_t unreadCount;        // Nombre de messages non lus
    uint32_t lastActivity;      // Derniere activite
} Conversation_t;

// ============================================
// STRUCTURE POUR LE FRAGMENTEUR DE SMS LONGS
// ============================================
typedef struct {
    uint16_t messageId;         // ID commun a tous les fragments
    uint8_t totalFragments;     // Nombre total de fragments
    uint8_t receivedFragments;  // Fragments recus
    char assemblyBuffer[500];   // Buffer d assemblage (max ~3x160)
    uint16_t totalLength;       // Longueur totale du message
    uint32_t startTime;         // Debut de la reception
    bool active;                // Assemblage en cours
} FragmentAssembly_t;

// ============================================
// CLASSE PRINCIPALE
// ============================================
class SMSManager {
    
private:
    // ============================================
    // ETAT ACTUEL
    // ============================================
    SMSState_t currentState;
    
    // ============================================
    // MESSAGE EN COURS DE COMPOSITION
    // ============================================
    char composingText[SMS_MAX_LENGTH + 1];  // Texte en cours
    char composingRecipient[16];             // Destinataire
    uint16_t composingIndex;                  // Position dans le texte
    
    // ============================================
    // CONVERSATIONS STOCKEES
    // ============================================
    Conversation_t conversations[SMS_MAX_CONVERSATIONS];
    uint8_t conversationCount;
    uint8_t selectedConversation;    // Conversation affichee
    
    // ============================================
    // FRAGMENTATION (messages > 160 car.)
    // ============================================
    FragmentAssembly_t fragmentAssembly;
    
    // ============================================
    // COMPTEURS
    // ============================================
    uint16_t nextMessageId;         // Prochain ID de message
    uint8_t totalUnreadMessages;    // Messages non lus (tous contacts)
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Trouve une conversation existante ou en cree une
    Conversation_t* findOrCreateConversation(const char* contact);
    
    // Trouve une conversation par numero
    Conversation_t* findConversation(const char* contact);
    
    // Ajoute un message a une conversation
    bool addMessageToConversation(const char* contact, Message_t* message);
    
    // Genere un ID unique pour un message
    uint16_t generateMessageId(void);
    
    // Envoie un paquet SMS via LoRa
    bool sendSMSPacket(uint16_t targetId, uint16_t messageId, 
                      uint8_t fragmentNum, uint8_t totalFragments,
                      const char* text, uint8_t length);
    
    // Envoie un accuse de reception
    bool sendAcknowledgment(uint16_t senderId, uint16_t messageId);
    
    // Assemble un message long a partir de fragments
    bool assembleFragment(uint16_t messageId, uint8_t fragmentNum,
                         uint8_t totalFragments, const char* data, uint8_t length);
    
    // Libere les ressources d assemblage
    void clearFragmentAssembly(void);
    
    // Formate la date/heure pour affichage
    void formatTimestamp(uint32_t timestamp, char* buffer, uint8_t bufferSize);
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    SMSManager();
    void init(void);
    
    // ============================================
    // COMPOSITION DE MESSAGE
    // ============================================
    
    // Commence un nouveau message
    void newMessage(void);
    
    // Definit le destinataire
    void setRecipient(const char* recipient);
    
    // Ajoute un caractere au texte
    void addChar(char c);
    
    // Supprime le dernier caractere
    void deleteChar(void);
    
    // Efface tout le message
    void clearMessage(void);
    
    // Retourne le texte en cours
    const char* getComposingText(void);
    
    // Retourne la longueur actuelle
    uint16_t getComposingLength(void);
    
    // Retourne le nombre de caracteres restants
    uint16_t getRemainingChars(void);
    
    // ============================================
    // ENVOI DE MESSAGE
    // ============================================
    
    // Envoie le message en cours
    bool sendMessage(void);
    
    // Reessaie d envoyer le dernier message echoue
    bool retrySend(void);
    
    // Retourne l etat actuel
    SMSState_t getState(void);
    
    // ============================================
    // RECEPTION DE MESSAGE
    // ============================================
    
    // Traite un paquet SMS recu
    void handleSMSPacket(uint16_t senderId, uint8_t* payload, uint8_t payloadSize);
    
    // Traite un accuse de reception
    void handleAcknowledgment(uint16_t senderId, uint16_t messageId);
    
    // Verifie si de nouveaux messages sont arrives
    bool hasNewMessages(void);
    
    // ============================================
    // CONVERSATIONS
    // ============================================
    
    // Retourne le nombre de conversations
    uint8_t getConversationCount(void);
    
    // Retourne une conversation par index
    Conversation_t* getConversation(uint8_t index);
    
    // Retourne l apercu d une conversation (dernier message)
    const char* getConversationPreview(uint8_t index);
    
    // Selectionne une conversation pour l afficher
    void selectConversation(uint8_t index);
    
    // Retourne la conversation selectionnee
    Conversation_t* getSelectedConversation(void);
    
    // Supprime une conversation
    void deleteConversation(uint8_t index);
    
    // ============================================
    // NOTIFICATIONS
    // ============================================
    
    // Retourne le nombre total de messages non lus
    uint8_t getUnreadCount(void);
    
    // Marque une conversation comme lue
    void markAsRead(uint8_t index);
    
    // Marque tous les messages comme lus
    void markAllAsRead(void);
    
    // ============================================
    // STOCKAGE
    // ============================================
    
    // Sauvegarde les conversations
    void save(void);
    
    // Charge les conversations
    void load(void);
    
    // Efface toutes les conversations
    void clearAll(void);
};

#endif // SMS_MANAGER_H