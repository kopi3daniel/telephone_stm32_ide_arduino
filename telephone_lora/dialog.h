/**
 * ---------------------------------------------------------------------------
 * dialog.h - Boites de dialogue pour le Telephone LoRa
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Ce widget gere toutes les boites de dialogue :
 * - Alerte simple (message + OK)
 * - Confirmation (Oui/Non)
 * - Saisie de texte
 * - Barre de progression
 * - Messages d erreur
 * - Alertes batterie faible
 */

#ifndef DIALOG_H
#define DIALOG_H

#include <Arduino.h>
#include "config.h"

// ============================================
// TYPES DE DIALOGUES
// ============================================
typedef enum {
    DIALOG_TYPE_ALERT,          // Message + bouton OK
    DIALOG_TYPE_CONFIRM,        // Question + Oui/Non
    DIALOG_TYPE_INPUT,          // Saisie de texte
    DIALOG_TYPE_PROGRESS,       // Barre de progression
    DIALOG_TYPE_TOAST,          // Message temporaire (2 secondes)
    DIALOG_TYPE_WARNING,        // Avertissement
    DIALOG_TYPE_ERROR           // Erreur
} DialogType_t;

// ============================================
// RESULTAT DU DIALOGUE
// ============================================
typedef enum {
    DIALOG_RESULT_NONE,         // Pas encore de reponse
    DIALOG_RESULT_OK,           // OK / Oui
    DIALOG_RESULT_CANCEL,       // Annuler / Non
    DIALOG_RESULT_TIMEOUT       // Timeout depasse
} DialogResult_t;

// ============================================
// DIMENSIONS DES DIALOGUES
// ============================================
#define DIALOG_WIDTH            110     // Largeur du dialogue
#define DIALOG_HEIGHT           40      // Hauteur du dialogue
#define DIALOG_X                9       // Position X (centre)
#define DIALOG_Y                12      // Position Y (centre)
#define DIALOG_PADDING          4       // Marge interieure

// ============================================
// CLASSE DIALOG
// ============================================
class Dialog {
    
private:
    // ============================================
    // ETAT DU DIALOGUE
    // ============================================
    DialogType_t type;              // Type de dialogue
    DialogResult_t result;          // Resultat
    bool isVisible;                 // Visible ?
    bool needsRedraw;               // Doit etre redessine ?
    
    // ============================================
    // CONTENU DU DIALOGUE
    // ============================================
    char title[32];                 // Titre
    char message[64];               // Message (2 lignes max)
    char inputText[32];             // Texte saisi (pour INPUT)
    uint8_t inputIndex;             // Position saisie
    uint8_t inputMaxLength;         // Longueur max
    
    // ============================================
    // BARRE DE PROGRESSION
    // ============================================
    uint8_t progressPercent;        // Pourcentage (0-100)
    char progressLabel[32];         // Label de progression
    
    // ============================================
    // TIMER
    // ============================================
    uint32_t showStartTime;         // Debut d affichage
    uint32_t timeoutMs;             // Timeout (0 = pas de timeout)
    bool hasTimeout;                // Timeout active ?
    
    // ============================================
    // BOUTONS
    // ============================================
    bool showOkButton;              // Afficher bouton OK
    bool showCancelButton;          // Afficher bouton Annuler
    bool okSelected;                // Bouton OK selectionne ?
    
    // ============================================
    // ANIMATION
    // ============================================
    bool animate;                   // Animation activee ?
    uint8_t animFrame;              // Frame d animation
    uint32_t lastAnimUpdate;        // Derniere mise a jour
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Dessine le fond du dialogue
    void drawBackground(void);
    
    // Dessine le titre
    void drawTitle(void);
    
    // Dessine le message
    void drawMessage(void);
    
    // Dessine les boutons
    void drawButtons(void);
    
    // Dessine la barre de progression
    void drawProgressBar(void);
    
    // Dessine le champ de saisie
    void drawInputField(void);
    
    // Efface la zone du dialogue
    void clearDialogArea(void);
    
public:
    // ============================================
    // CONSTRUCTEUR
    // ============================================
    Dialog();
    
    // ============================================
    // MISE A JOUR
    // ============================================
    void update(void);
    void draw(void);
    
    // ============================================
    // AFFICHAGE DES DIALOGUES
    // ============================================
    
    // Affiche une alerte simple
    void showAlert(const char* title, const char* message);
    
    // Affiche une confirmation Oui/Non
    void showConfirm(const char* title, const char* message);
    
    // Affiche une saisie de texte
    void showInput(const char* title, uint8_t maxLength);
    
    // Affiche une barre de progression
    void showProgress(const char* label, uint8_t percent);
    
    // Affiche un message temporaire (toast)
    void showToast(const char* message);
    
    // Affiche un avertissement
    void showWarning(const char* message);
    
    // Affiche une erreur
    void showError(const char* message);
    
    // Affiche une alerte batterie faible
    void showLowBattery(uint8_t percent);
    
    // ============================================
    // GESTION DU DIALOGUE
    // ============================================
    
    // Ferme le dialogue
    void close(void);
    
    // Gere les touches
    uint8_t handleKeyPress(char key);
    
    // Verifie si le dialogue est visible
    bool isDialogVisible(void);
    
    // Retourne le resultat
    DialogResult_t getResult(void);
    
    // Met a jour la progression
    void updateProgress(uint8_t percent);
    
    // ============================================
    // SAISIE DE TEXTE
    // ============================================
    void inputAddChar(char c);
    void inputDeleteChar(void);
    const char* getInputText(void);
    void clearInput(void);
};

#endif // DIALOG_H