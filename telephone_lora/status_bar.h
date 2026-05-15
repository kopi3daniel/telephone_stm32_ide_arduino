/**

 * Ce widget gere l affichage de la barre d etat en haut de l ecran.
 * Elle affiche en permanence :
 * - La force du signal LoRa (icone + dBm)
 * - L heure actuelle (format 24h)
 * - Le niveau de batterie (icone + pourcentage)
 * - Les icones de notification (message, appel manque, silencieux, torche)
 * - Le mode de fonctionnement (normal, relais)
 */

#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <Arduino.h>
#include "config.h"

// ============================================
// DIMENSIONS DE LA BARRE D ETAT
// ============================================
#define STATUSBAR_HEIGHT        8       // Hauteur en pixels
#define STATUSBAR_WIDTH         128     // Largeur (plein ecran)

// ============================================
// POSITIONS DES ELEMENTS
// ============================================
#define SIGNAL_ICON_X           0       // Icone signal (gauche)
#define SIGNAL_TEXT_X           10      // Texte dBm
#define TIME_X                  50      // Heure (centre)
#define BATTERY_ICON_X          95      // Icone batterie
#define BATTERY_TEXT_X          105     // Pourcentage batterie
#define NOTIF_START_X           28      // Debut zone notifications
#define NOTIF_SPACING           8       // Espacement entre icones

// ============================================
// CLASSE STATUS BAR
// ============================================
class StatusBar {
    
private:
    // ============================================
    // DONNEES AFFICHEES
    // ============================================
    
    // Signal
    int8_t signalRSSI;              // Force du signal (-120 a 0 dBm)
    uint8_t signalBars;             // Nombre de barres (0-4)
    
    // Heure
    uint8_t currentHour;            // Heure (0-23)
    uint8_t currentMinute;          // Minute (0-59)
    bool clockVisible;              // Horloge visible ?
    
    // Batterie
    uint8_t batteryPercent;         // Pourcentage (0-100)
    bool batteryCharging;           // En charge ?
    bool batteryLow;                // Batterie faible (< 15%) ?
    
    // Notifications
    bool hasNewMessage;             // Nouveau message ?
    bool hasMissedCall;             // Appel manque ?
    bool silentMode;                // Mode silencieux ?
    bool torchOn;                   // Torche allumee ?
    bool isLocked;                  // Telephone verrouille ?
    
    // Reseau
    bool relayMode;                 // Mode relais actif ?
    bool networkConnected;          // Connecte au reseau ?
    
    // ============================================
    // ETAT INTERNE
    // ============================================
    bool needsRedraw;               // Doit etre redessinee ?
    uint32_t lastUpdate;            // Derniere mise a jour
    uint32_t lastBlink;             // Dernier clignotement
    bool blinkState;                // Etat du clignotement
    
    // ============================================
    // ICONES BITMAP (8x8 pixels)
    // ============================================
    
    // Signal (0 a 4 barres)
    static const uint8_t icon_signal_0[8];
    static const uint8_t icon_signal_1[8];
    static const uint8_t icon_signal_2[8];
    static const uint8_t icon_signal_3[8];
    static const uint8_t icon_signal_4[8];
    
    // Batterie (etats)
    static const uint8_t icon_battery_full[8];
    static const uint8_t icon_battery_75[8];
    static const uint8_t icon_battery_50[8];
    static const uint8_t icon_battery_25[8];
    static const uint8_t icon_battery_empty[8];
    static const uint8_t icon_battery_charge[8];
    
    // Notifications
    static const uint8_t icon_message[8];
    static const uint8_t icon_call_missed[8];
    static const uint8_t icon_silent[8];
    static const uint8_t icon_torch[8];
    static const uint8_t icon_lock[8];
    static const uint8_t icon_relay[8];
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Convertit le RSSI en nombre de barres
    uint8_t rssiToBars(int8_t rssi);
    
    // Selectionne la bonne icone de signal
    const uint8_t* getSignalIcon(void);
    
    // Selectionne la bonne icone de batterie
    const uint8_t* getBatteryIcon(void);
    
    // Dessine un element de la barre
    void drawSignal(void);
    void drawClock(void);
    void drawBattery(void);
    void drawNotifications(void);
    
public:
    // ============================================
    // CONSTRUCTEUR
    // ============================================
    StatusBar();
    
    // ============================================
    // INITIALISATION
    // ============================================
    void init(void);
    
    // ============================================
    // MISE A JOUR
    // ============================================
    
    // Appele periodiquement pour rafraichir
    void update(void);
    
    // Force le redessin
    void forceRedraw(void);
    
    // ============================================
    // SETTERS (mise a jour des donnees)
    // ============================================
    
    void setSignal(int8_t rssi);
    void setTime(uint8_t hour, uint8_t minute);
    void setBattery(uint8_t percent, bool charging);
    void setNewMessage(bool has);
    void setMissedCall(bool has);
    void setSilentMode(bool silent);
    void setTorchOn(bool on);
    void setLocked(bool locked);
    void setRelayMode(bool relay);
    void setNetworkConnected(bool connected);
    
    // ============================================
    // GETTERS
    // ============================================
    
    int8_t getSignalRSSI(void);
    uint8_t getBatteryPercent(void);
    bool hasNewMessageIndicator(void);
    bool hasMissedCallIndicator(void);
    
    // ============================================
    // AFFICHAGE
    // ============================================
    
    // Dessine la barre complete (appele par l ecran parent)
    void draw(void);
};

#endif // STATUS_BAR_H