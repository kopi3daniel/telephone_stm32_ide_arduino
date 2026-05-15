

#ifndef INPUT_FIELD_H
#define INPUT_FIELD_H

#include <Arduino.h>
#include "config.h"

// ============================================
// CONFIGURATION DU CHAMP
// ============================================
#define INPUT_MAX_LENGTH        160     // Longueur max par defaut
#define INPUT_CURSOR_BLINK_MS   500     // Clignotement curseur (ms)
#define INPUT_PREVIEW_TIMEOUT_MS 800    // Timeout previsualisation

// ============================================
// CLASSE INPUT FIELD
// ============================================
class InputField {
    
private:
    // ============================================
    // DONNEES DU CHAMP
    // ============================================
    char text[INPUT_MAX_LENGTH + 1];    // Texte saisi (+1 pour '\0')
    uint8_t textLength;                  // Longueur actuelle
    uint8_t maxLength;                   // Longueur maximale
    uint8_t cursorPos;                   // Position du curseur
    
    // ============================================
    // POSITION ET TAILLE
    // ============================================
    uint8_t fieldX;                      // Position X
    uint8_t fieldY;                      // Position Y
    uint8_t fieldWidth;                  // Largeur en pixels
    uint8_t fieldHeight;                 // Hauteur en pixels
    
    // ============================================
    // ETAT DU CURSEUR
    // ============================================
    bool cursorVisible;                  // Curseur visible ?
    uint32_t lastCursorToggle;           // Dernier clignotement
    bool needsRedraw;                    // Redessiner ?
    
    // ============================================
    // PREVISUALISATION MULTI-TAP
    // ============================================
    char previewChar;                    // Caractere en previsualisation
    uint32_t previewStartTime;           // Debut previsualisation
    bool previewActive;                  // Previsualisation active ?
    
    // ============================================
    // MODE DE SAISIE
    // ============================================
    char modeIndicator[4];               // "abc", "ABC", "123"
    
    // ============================================
    // OPTIONS D AFFICHAGE
    // ============================================
    bool showBorder;                     // Afficher la bordure ?
    bool showCounter;                    // Afficher le compteur ?
    bool showMode;                       // Afficher le mode ?
    bool isActive;                       // Champ actif ?
    
    // ============================================
    // SCROLL HORIZONTAL
    // ============================================
    uint8_t scrollOffset;                // Decalage pour texte long
    uint8_t visibleChars;                // Caracteres visibles
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Dessine le curseur
    void drawCursor(void);
    
    // Dessine le texte visible
    void drawVisibleText(void);
    
    // Dessine le compteur de caracteres
    void drawCounter(void);
    
    // Dessine l indicateur de mode
    void drawModeIndicator(void);
    
    // Calcule le nombre de caracteres visibles
    uint8_t calculateVisibleChars(void);
    
    // Ajuste le scroll pour que le curseur soit visible
    void adjustScroll(void);
    
public:
    // ============================================
    // CONSTRUCTEUR
    // ============================================
    InputField();
    
    // ============================================
    // INITIALISATION
    // ============================================
    void init(void);
    
    // ============================================
    // CONFIGURATION
    // ============================================
    
    // Definit la position et taille
    void setBounds(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
    
    // Definit la longueur maximale
    void setMaxLength(uint8_t max);
    
    // Affiche/masque la bordure
    void showFieldBorder(bool show);
    
    // Affiche/masque le compteur
    void showCharCounter(bool show);
    
    // Affiche/masque le mode
    void showModeDisplay(bool show);
    
    // Definit le mode de saisie
    void setMode(const char* mode);
    
    // ============================================
    // EDITION DU TEXTE
    // ============================================
    
    // Ajoute un caractere a la position du curseur
    void addChar(char c);
    
    // Supprime le caractere avant le curseur
    void deleteChar(void);
    
    // Supprime le caractere apres le curseur
    void deleteCharForward(void);
    
    // Efface tout le texte
    void clear(void);
    
    // Deplace le curseur a gauche
    void cursorLeft(void);
    
    // Deplace le curseur a droite
    void cursorRight(void);
    
    // Deplace le curseur au debut
    void cursorHome(void);
    
    // Deplace le curseur a la fin
    void cursorEnd(void);
    
    // ============================================
    // PREVISUALISATION MULTI-TAP
    // ============================================
    
    // Affiche un caractere en previsualisation
    void showPreview(char c);
    
    // Confirme le caractere previsualise
    void confirmPreview(void);
    
    // Annule la previsualisation
    void cancelPreview(void);
    
    // ============================================
    // CONTENU
    // ============================================
    
    // Retourne le texte saisi
    const char* getText(void);
    
    // Retourne la longueur du texte
    uint8_t getLength(void);
    
    // Retourne le nombre de caracteres restants
    uint8_t getRemainingChars(void);
    
    // Definit le texte (pre-remplissage)
    void setText(const char* newText);
    
    // ============================================
    // ETAT
    // ============================================
    
    // Active/desactive le champ
    void setActive(bool active);
    
    // Verifie si le champ est actif
    bool isActiveField(void);
    
    // Verifie si le champ est vide
    bool isEmpty(void);
    
    // Verifie si le champ est plein
    bool isFull(void);
    
    // ============================================
    // MISE A JOUR ET AFFICHAGE
    // ============================================
    
    // Mise a jour periodique (curseur, previsualisation)
    void update(void);
    
    // Dessine le champ complet
    void draw(void);
    
    // Force le redessin
    void forceRedraw(void);
};

#endif // INPUT_FIELD_H