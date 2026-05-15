/**

 * Cet ecran gere la composition des numeros de telephone.
 * Fonctionnalites :
 * - Affichage du numero compose en temps reel
 * - Saisie des chiffres (0-9, *, #, +)
 * - Correction (effacer le dernier chiffre)
 * - Suggestion de contacts pendant la frappe
 * - Lancement d appel (bouton VERT)
 * - Retour a l accueil (bouton ROUGE)
 * - Affichage du contact si le numero est reconnu
 */

#ifndef DIALER_SCREEN_H
#define DIALER_SCREEN_H

#include <Arduino.h>
#include "config.h"

// ============================================
// ETATS DU COMPOSEUR
// ============================================
typedef enum {
    DIALER_STATE_IDLE,          // Attente saisie
    DIALER_STATE_TYPING,        // Saisie en cours
    DIALER_STATE_SUGGESTING,    // Suggestions affichees
    DIALER_STATE_CALLING,       // Appel lance
    DIALER_STATE_ERROR          // Erreur (numero invalide)
} DialerState_t;

// ============================================
// NOMBRE MAX DE SUGGESTIONS
// ============================================
#define MAX_SUGGESTIONS     5       // Nombre max de suggestions affichees

// ============================================
// CLASSE DIALER SCREEN
// ============================================
class DialerScreen {
    
private:
    // ============================================
    // ETAT DU COMPOSEUR
    // ============================================
    DialerState_t state;            // Etat actuel
    bool isActive;                  // Ecran actif ?
    uint32_t lastUpdate;            // Derniere mise a jour
    
    // ============================================
    // NUMERO EN COURS
    // ============================================
    char dialNumber[16];            // Numero compose (max 15 chiffres + \0)
    uint8_t dialIndex;              // Position actuelle dans le numero
    bool cursorVisible;             // Curseur clignotant visible ?
    uint32_t lastCursorToggle;      // Dernier clignotement du curseur
    
    // ============================================
    // SUGGESTIONS DE CONTACTS
    // ============================================
    struct Suggestion {
        char name[32];              // Nom du contact
        char number[16];            // Numero du contact
        bool isFavorite;            // Contact favori ?
    };
    
    Suggestion suggestions[MAX_SUGGESTIONS];  // Liste des suggestions
    uint8_t suggestionCount;                  // Nombre de suggestions
    uint8_t selectedSuggestion;               // Suggestion selectionnee
    
    // ============================================
    // INFORMATIONS DU CONTACT RECONNU
    // ============================================
    char recognizedName[32];        // Nom si le numero complet est reconnu
    bool isRecognized;              // Numero reconnu dans les contacts ?
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Dessine l ecran complet
    
    // Dessine la zone du numero
    void drawNumber(void);
    
    // Dessine les suggestions
    void drawSuggestions(void);
    
    // Dessine le nom du contact reconnu
    void drawRecognizedName(void);
    
    // Dessine la barre d instructions
    void drawInstructions(void);
    
    // Dessine le curseur clignotant
    void drawCursor(void);
    
    // Ajoute un chiffre au numero
    bool addDigit(char digit);
    
    // Efface le dernier chiffre
    void deleteDigit(void);
    
    // Efface tout le numero (appui long)
    void clearAll(void);
    
    // Cherche des suggestions de contacts
    void findSuggestions(void);
    
    // Verifie si le numero complet correspond a un contact
    void checkRecognizedNumber(void);
    
    // Formate le numero pour l affichage (espaces)
    void formatNumberForDisplay(char* output, const char* input);
    
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    DialerScreen();
    void init(void);
    
    // ============================================
    // CYCLE DE VIE DE L ECRAN
    // ============================================
    void onEnter(void);
    void onExit(void);
    void update(void);
    void draw(void);

    // ============================================
    // GESTION DES TOUCHES
    // ============================================
    
    // Traite une touche pressee
    // Retourne :
    //   0 = l ecran gere la touche
    //   1 = lancer l appel (numero dans getNumber())
    //   255 = retour a l accueil
    uint8_t handleKeyPress(char key);
    
    // ============================================
    // INFORMATIONS
    // ============================================
    
    // Retourne le numero compose
    const char* getNumber(void);
    
    // Retourne la longueur du numero
    uint8_t getNumberLength(void);
    
    // Verifie si un numero est en cours de saisie
    bool hasNumber(void);
    
    // Retourne l etat actuel
    DialerState_t getState(void);
    
    // Verifie si l ecran est actif
    bool isActiveScreen(void);
    
    // ============================================
    // ACTIONS EXTERNES
    // ============================================
    
    // Affiche une erreur (numero invalide, pas de reseau...)
    void showError(const char* message);
    
    // Reinitialise le composeur
    void reset(void);
    
    // Pre-remplit un numero (rappel depuis journal)
    void setNumber(const char* number);
};

#endif // DIALER_SCREEN_H