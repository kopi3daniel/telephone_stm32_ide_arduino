/**
 * ---------------------------------------------------------------------------
 * sms_screen.h - Ecran de messagerie SMS du Telephone LoRa
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Cet ecran gere toute l interface des messages texte :
 * - Liste des conversations (apercu du dernier message)
 * - Vue d une conversation (bulles de dialogue)
 * - Editeur de message avec multi-tap
 * - Envoi et reception de SMS
 * - Notifications de nouveaux messages
 * - Horodatage des messages
 */

#ifndef SMS_SCREEN_H
#define SMS_SCREEN_H

#include <Arduino.h>
#include "config.h"

// ============================================
// MODES DE L ECRAN SMS
// ============================================
typedef enum {
    SMS_MODE_LIST,              // Liste des conversations
    SMS_MODE_CONVERSATION,      // Vue d une conversation
    SMS_MODE_EDITOR,            // Editeur de nouveau message
    SMS_MODE_SENDING,           // Envoi en cours
    SMS_MODE_SENT,              // Message envoye (confirmation)
    SMS_MODE_FAILED             // Echec d envoi
} SMSMode_t;

// ============================================
// STRUCTURE D UN MESSAGE (AFFICHAGE)
// ============================================
typedef struct {
    char text[SMS_MAX_LENGTH + 1];  // Texte du message
    bool outgoing;                   // true = envoye, false = recu
    uint32_t timestamp;              // Horodatage
    bool read;                       // Lu / non lu
} SMSMessage_t;

// ============================================
// STRUCTURE D UNE CONVERSATION
// ============================================
typedef struct {
    char contactName[32];            // Nom du contact
    char contactNumber[16];          // Numero du contact
    SMSMessage_t messages[SMS_MAX_PER_CONVERSATION]; // Messages
    uint8_t messageCount;            // Nombre de messages
    uint8_t unreadCount;             // Messages non lus
    uint32_t lastActivity;           // Derniere activite
} SMSConversation_t;

// ============================================
// TAILLES MAXIMALES
// ============================================
#define SMS_MAX_CONVERSATIONS_DISPLAY 20
#define SMS_MESSAGES_PER_PAGE         5

// ============================================
// CLASSE SMS SCREEN
// ============================================
class SMSScreen {
    
private:
    // ============================================
    // ETAT DE L ECRAN
    // ============================================
    SMSMode_t mode;                 // Mode actuel
    bool isActive;                  // Ecran actif ?
    uint32_t lastUpdate;            // Derniere mise a jour
    
    // ============================================
    // CONVERSATIONS
    // ============================================
    SMSConversation_t conversations[SMS_MAX_CONVERSATIONS_DISPLAY];
    uint8_t conversationCount;
    uint8_t selectedConversation;   // Index conversation selectionnee
    uint8_t conversationScroll;     // Defilement dans la liste
    
    // ============================================
    // MESSAGES D UNE CONVERSATION
    // ============================================
    uint8_t messageScroll;          // Defilement des messages
    uint8_t selectedMessage;        // Message selectionne
    
    // ============================================
    // EDITEUR DE MESSAGE
    // ============================================
    char editText[SMS_MAX_LENGTH + 1];  // Texte en cours
    char editRecipient[16];             // Destinataire
    uint8_t editIndex;                  // Position curseur
    uint8_t charsRemaining;             // Caracteres restants
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Charge les conversations
    void loadConversations(void);
    
    // Sauvegarde les conversations
    void saveConversations(void);
    
    // Dessin selon le mode
    void drawListMode(void);
    void drawConversationMode(void);
    void drawEditorMode(void);
    void drawSendingMode(void);
    void drawSentMode(void);
    void drawFailedMode(void);
    
    // Dessine un apercu de conversation
    void drawConversationPreview(uint8_t index, uint8_t y, bool selected);
    
    // Dessine une bulle de message
    void drawMessageBubble(uint8_t index, uint8_t y, SMSMessage_t* msg);
    
    // Dessine l en-tete
    void drawHeader(const char* title);
    
    // Dessine les instructions
    void drawInstructions(const char* left, const char* center, const char* right);
    
    // Dessine le compteur de caracteres
    void drawCharCounter(void);
    
    // Formate l horodatage
    void formatTime(char* buffer, uint32_t timestamp);
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    SMSScreen();
    void init(void);
    
    // ============================================
    // CYCLE DE VIE
    // ============================================
    void onEnter(void);
    void onExit(void);
    void update(void);
    void draw(void);
    
    // ============================================
    // GESTION DES TOUCHES
    // ============================================
    uint8_t handleKeyPress(char key);
    
    // ============================================
    // ACTIONS
    // ============================================
    
    // Ouvre une conversation
    void openConversation(uint8_t index);
    
    // Nouveau message
    void newMessage(const char* recipient);
    
    // Envoie le message en cours
    bool sendMessage(void);
    
    // Ajoute un message recu
    void addReceivedMessage(const char* sender, const char* text);
    
    // Marque une conversation comme lue
    void markAsRead(uint8_t index);
    
    // Supprime une conversation
    void deleteConversation(uint8_t index);
    
    // ============================================
    // INFORMATIONS
    // ============================================
    uint8_t getConversationCount(void);
    uint8_t getUnreadCount(void);
    const char* getEditText(void);
    bool isActiveScreen(void);
};

#endif // SMS_SCREEN_H