/**

 * Cet ecran est la page principale affichee quand le telephone est allume.
 * Il montre :
 * - L horloge en grand
 * - La date
 * - Les icones de menu principal (Appels, SMS, Contacts, Reglages)
 * - Les notifications (messages non lus, appels manques)
 * - Le niveau de batterie
 * - La force du signal LoRa
 */

#ifndef HOME_SCREEN_H
#define HOME_SCREEN_H

#include <Arduino.h>
#include "config.h"

// ============================================
// ELEMENTS DU MENU D ACCUEIL
// ============================================
typedef enum {
    HOME_MENU_CALLS,        // Journal d appels
    HOME_MENU_MESSAGES,     // Messages SMS
    HOME_MENU_CONTACTS,     // Repertoire
    HOME_MENU_SETTINGS,     // Parametres
    HOME_MENU_COUNT         // Nombre total d elements (4)
} HomeMenuItem_t;

// ============================================
// CLASSE HOME SCREEN
// ============================================
class HomeScreen {
    
private:
    // ============================================
    // ETAT DE L ECRAN
    // ============================================
    HomeMenuItem_t selectedItem;    // Element selectionne
    bool isActive;                  // Ecran actif ?
    uint32_t lastUpdate;            // Derniere mise a jour
    
    // ============================================
    // DONNEES AFFICHEES
    // ============================================
    uint8_t currentHour;            // Heure actuelle
    uint8_t currentMinute;          // Minute actuelle
    uint8_t currentDay;             // Jour
    uint8_t currentMonth;           // Mois
    char    dayName[4];             // Nom du jour (Lun, Mar...)
    
    int8_t  signalRSSI;             // Force du signal (dBm)
    uint8_t batteryPercent;         // Pourcentage batterie
    bool    batteryCharging;        // En charge ?
    
    uint8_t unreadMessages;         // Messages non lus
    uint8_t missedCalls;            // Appels manques
    bool    silentMode;             // Mode silencieux
    bool    torchOn;                // Torche allumee
    
    // ============================================
    // COORDONNEES DES ICONES
    // ============================================
    static const uint8_t ICON_SIZE = 16;      // Taille des icones
    static const uint8_t ICON_SPACING = 8;    // Espacement
    
    // Positions des 4 icones du menu
    struct IconPosition {
        uint8_t x;
        uint8_t y;
    };
    
    IconPosition iconPositions[4];
    
    // ============================================
    // BITMAPS DES ICONES DU MENU
    // ============================================
    
    // // Icone Appels (combine telephonique)
    // static const uint8_t icon_calls[16];
    
    // // Icone Messages (bulle de dialogue)
    // static const uint8_t icon_messages[16];
    
    // // Icone Contacts (silhouette)
    // static const uint8_t icon_contacts[16];
    
    // // Icone Reglages (engrenage)
    // static const uint8_t icon_settings[16];


        // Icone Appels (combine telephonique)
    static const uint8_t icon_calls[32];
    
    // Icone Messages (bulle de dialogue)
    static const uint8_t icon_messages[32];
    
    // Icone Contacts (silhouette)
    static const uint8_t icon_contacts[32];
    
    // Icone Reglages (engrenage)
    static const uint8_t icon_settings[32];
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Dessine l arriere-plan
    void drawBackground(void);
    
    // Dessine l horloge
    void drawClock(void);
    
    // Dessine la date
    void drawDate(void);
    
    // Dessine les icones du menu
    void drawMenuIcons(void);
    
    // Dessine une icone avec son etiquette
    void drawMenuIcon(uint8_t index, const uint8_t* bitmap, const char* label);
    
    // Surligne l icone selectionnee
    void highlightSelected(void);
    
    // Dessine les badges de notification
    void drawNotificationBadges(void);
    
    // Dessine un badge circulaire avec un nombre
    void drawBadge(uint8_t x, uint8_t y, uint8_t count);
    
    // Met a jour l horloge interne
    void updateClock(void);
    
    // Convertit un numero de mois en nom
    const char* getMonthName(uint8_t month);
    
    // Tableau des noms de jours
    static const char* dayNames[7];
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    HomeScreen();
    void init(void);
    
    // ============================================
    // CYCLE DE VIE DE L ECRAN
    // ============================================
    
    // Appele quand l ecran devient actif
    void onEnter(void);
    
    // Appele quand l ecran n est plus actif
    void onExit(void);
    
    // Mise a jour periodique (appele dans la boucle)
    void update(void);
    
    // Dessine tout l ecran
    void draw(void);
    
    // ============================================
    // GESTION DES TOUCHES
    // ============================================
    
    // Touche pressee : retourne l action a effectuer
    uint8_t handleKeyPress(char key);
    
    // Navigation
    void moveSelectionLeft(void);
    void moveSelectionRight(void);
    void moveSelectionUp(void);
    void moveSelectionDown(void);
    void selectItem(void);
    
    // ============================================
    // MISE A JOUR DES DONNEES
    // ============================================
    
    // Met a jour le signal
    void setSignal(int8_t rssi);
    
    // Met a jour la batterie
    void setBattery(uint8_t percent, bool charging);
    
    // Met a jour l heure
    void setTime(uint8_t hour, uint8_t minute);
    
    // Met a jour la date
    void setDate(uint8_t day, uint8_t month, const char* dayName);
    
    // Definit le nombre de messages non lus
    void setUnreadMessages(uint8_t count);
    
    // Definit le nombre d appels manques
    void setMissedCalls(uint8_t count);
    
    // Active/desactive le mode silencieux
    void setSilentMode(bool silent);
    
    // Active/desactive l icone torche
    void setTorchOn(bool on);
    
    // ============================================
    // INFORMATIONS
    // ============================================
    
    // Retourne l element selectionne
    HomeMenuItem_t getSelectedItem(void);
    
    // Verifie si l ecran est actif
    bool isActiveScreen(void);
};

#endif // HOME_SCREEN_H