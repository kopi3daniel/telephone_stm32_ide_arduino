/**
  
 * Cet ecran gere tous les reglages du telephone :
 * - Son (volume, sonnerie, vibreur, mode silencieux)
 * - Ecran (luminosite, timeout)
 * - Reseau (frequence, puissance, ID)
 * - Securite (code PIN, chiffrement)
 * - Systeme (version, reset, a propos)
 * - Energie (mode economie, statut batterie)
 */

#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include <Arduino.h>
#include "config.h"

// ============================================
// CATEGORIES DE REGLAGES
// ============================================
typedef enum {
    SETTINGS_CATEGORY_SOUND,        // Son et vibrations
    SETTINGS_CATEGORY_DISPLAY,      // Affichage
    SETTINGS_CATEGORY_NETWORK,      // Reseau LoRa
    SETTINGS_CATEGORY_SECURITY,     // Securite
    SETTINGS_CATEGORY_POWER,        // Energie
    SETTINGS_CATEGORY_SYSTEM,       // Systeme
    SETTINGS_CATEGORY_COUNT
} SettingsCategory_t;

// ============================================
// MODES DE L ECRAN REGLAGES
// ============================================
typedef enum {
    SETTINGS_MODE_MAIN_MENU,        // Menu principal des categories
    SETTINGS_MODE_SUB_MENU,         // Sous-menu d une categorie
    SETTINGS_MODE_SLIDER,           // Reglage par curseur
    SETTINGS_MODE_SELECT,           // Selection dans une liste
    SETTINGS_MODE_PIN_ENTRY,        // Saisie du code PIN
    SETTINGS_MODE_CONFIRM           // Confirmation (reset, etc.)
} SettingsMode_t;

// ============================================
// OPTIONS DE CHAQUE CATEGORIE
// ============================================
#define MAX_OPTIONS_PER_CATEGORY   8

// typedef struct {
//     const char* name;               // Nom de l option
//     uint8_t value;                  // Valeur actuelle
//     uint8_t minValue;               // Valeur minimum
//     uint8_t maxValue;               // Valeur maximum
//     const char** choices;           // Choix possibles (pour SELECT)
//     uint8_t choiceCount;            // Nombre de choix
// } SettingOption_t;

typedef struct {
    const char* name;
    uint16_t value;                  // uint8_t → uint16_t
    uint16_t minValue;               // uint8_t → uint16_t
    uint16_t maxValue;               // uint8_t → uint16_t
    const char** choices;
    uint8_t choiceCount;
} SettingOption_t;

// ============================================
// CLASSE SETTINGS SCREEN
// ============================================
class SettingsScreen {
    
private:
    // ============================================
    // ETAT DE L ECRAN
    // ============================================
    SettingsMode_t mode;            // Mode actuel
    SettingsCategory_t category;    // Categorie selectionnee
    bool isActive;                  // Ecran actif ?
    uint32_t lastUpdate;            // Derniere mise a jour
    
    // ============================================
    // NAVIGATION DANS LES MENUS
    // ============================================
    uint8_t selectedItem;           // Element selectionne
    uint8_t itemCount;              // Nombre d elements dans le menu
    uint8_t scrollOffset;           // Defilement
    uint8_t visibleCount;           // Nombre visible
    
    // ============================================
    // OPTIONS DE REGLAGE
    // ============================================
    SettingOption_t options[MAX_OPTIONS_PER_CATEGORY];
    uint8_t optionCount;
    
    // ============================================
    // VALEURS DES REGLAGES
    // ============================================
    
    // Son
    uint8_t ringVolume;             // Volume sonnerie (0-10)
    uint8_t ringTone;               // Sonnerie (0-9)
    bool vibrateEnabled;            // Vibreur active ?
    bool silentMode;                // Mode silencieux ?
    
    // Ecran
    uint8_t brightness;             // Luminosite (10-255)
    uint16_t screenTimeout;          // Extinction ecran (10-300 sec)
    
    // Reseau
    uint8_t frequencyIndex;         // Index frequence (0=868, 1=915, 2=433)
    uint8_t txPower;                // Puissance (2-20 dBm)
    uint16_t phoneId;               // ID du telephone
    
    // Securite
    char pinCode[5];                // Code PIN (4 chiffres)
    bool lockEnabled;               // Verrouillage active ?
    bool encryptionEnabled;         // Chiffrement active ?
    
    // Energie
    bool powerSavingEnabled;        // Mode economie ?
    uint8_t sleepTimeout;           // Delai veille (1-60 min)
    
    // ============================================
    // SAISIE PIN
    // ============================================
    char pinEntry[5];               // PIN en cours de saisie
    uint8_t pinEntryIndex;          // Position saisie
    bool pinConfirmMode;            // Mode confirmation (nouveau PIN)
    char pinNew[5];                 // Nouveau PIN temporaire
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Charge les reglages sauvegardes
    void loadSettings(void);
    
    // Sauvegarde les reglages
    void saveSettings(void);
    
    // Construit le menu pour une categorie
    void buildCategoryMenu(SettingsCategory_t cat);
    
    // Dessin selon le mode
    void drawMainMenu(void);
    void drawSubMenu(void);
    void drawSlider(void);
    void drawSelectList(void);
    void drawPinEntry(void);
    void drawConfirm(void);
    
    // Dessine un element de menu
    void drawMenuItem(uint8_t index, uint8_t y, bool selected);
    
    // Dessine un curseur de reglage
    void drawSliderBar(uint8_t y, uint8_t value, uint8_t maxValue);
    
    // Applique un reglage
    void applySetting(void);
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    SettingsScreen();
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
    // ACCES AUX REGLAGES
    // ============================================
    uint8_t getVolume(void);
    uint8_t getBrightness(void);
    bool isSilentMode(void);
    bool isVibrateEnabled(void);
    uint8_t getFrequencyIndex(void);
    uint8_t getTxPower(void);
    bool isEncryptionEnabled(void);
    
    // ============================================
    // INFORMATIONS
    // ============================================
    bool isActiveScreen(void);
};

#endif // SETTINGS_SCREEN_H