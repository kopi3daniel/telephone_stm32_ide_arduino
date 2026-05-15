/**
 * Bibliothèques requises :
 * - Adafruit GFX Library (par Adafruit)
 * - Adafruit SSD1306 (par Adafruit)
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// Bibliothèques Adafruit pour l'OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================
// ÉTATS DE L'INTERFACE
// ============================================
typedef enum {
    SCREEN_HOME,            // Écran d'accueil
    SCREEN_DIALER,          // Composeur numérique
    SCREEN_CALL_ACTIVE,     // Appel en cours
    SCREEN_INCOMING_CALL,   // Appel entrant
    SCREEN_CALL_ENDED,      // Appel terminé
    SCREEN_MESSAGE_EDITOR,  // Éditeur de SMS
    SCREEN_MESSAGE_LIST,    // Liste des conversations
    SCREEN_CONTACTS_LIST,   // Liste des contacts
    SCREEN_CONTACT_VIEW,    // Détail d'un contact
    SCREEN_CONTACT_EDIT,    // Édition d'un contact
    SCREEN_SETTINGS_MENU,   // Menu des réglages
    SCREEN_CALL_LOG,        // Journal d'appels
    SCREEN_LOCK             // Écran de verrouillage
} ScreenType_t;

// ============================================
// STRUCTURE POUR LES ICÔNES BITMAP
// ============================================
typedef struct {
    const uint8_t* bitmap;
    uint8_t width;
    uint8_t height;
} Icon_t;

class Display {
    
private:
    // Objet écran Adafruit
    Adafruit_SSD1306 oled;
    
    // État actuel
    ScreenType_t currentScreen;
    bool screenOn;
    uint8_t brightness;
    
    // Dimensions
    static const uint8_t W = OLED_WIDTH;
    static const uint8_t H = OLED_HEIGHT;
    
    // ============================================
    // ICÔNES BITMAP PRÉDÉFINIES
    // ============================================
    static const uint8_t signal_0[8];
    static const uint8_t signal_1[8];
    static const uint8_t signal_2[8];
    static const uint8_t signal_3[8];
    static const uint8_t signal_4[8];
    
    static const uint8_t battery_full[8];
    static const uint8_t battery_75[8];
    static const uint8_t battery_50[8];
    static const uint8_t battery_25[8];
    static const uint8_t battery_empty[8];
    static const uint8_t battery_charge[8];
    
    static const uint8_t icon_message[8];
    static const uint8_t icon_call_missed[8];
    static const uint8_t icon_torch[8];
    static const uint8_t icon_lock[8];
    static const uint8_t icon_silent[8];
    static const uint8_t icon_mute[8];
    static const uint8_t icon_speaker[8];
    
    // Variables d'état pour la barre d'état
    int8_t currentRssi;
    uint8_t currentBatteryPercent;
    bool currentBatteryCharging;
    uint8_t currentHour;
    uint8_t currentMinute;
    bool missedCallIcon;
    bool newMessageIcon;
    bool silentIcon;
    bool torchIcon;
    
    // Fonctions privées
    
    void drawTitleBar(const char* title);
    void drawBottomBar(const char* left, const char* center, const char* right);
    // void drawIcon(uint8_t x, uint8_t y, const uint8_t* bitmap, uint8_t w, uint8_t h);
    // void drawCenteredText(uint8_t y, const char* text, uint8_t size);
    
    // Méthodes privées pour les icônes
    const uint8_t* getSignalIcon(int8_t rssi);
    const uint8_t* getBatteryIcon(uint8_t percent, bool charging);
    
public:

// Ces methodes doivent etre publiques pour HomeScreen
    void drawCenteredText(uint8_t y, const char* text, uint8_t size);
    void drawIcon(uint8_t x, uint8_t y, const uint8_t* bitmap, uint8_t w, uint8_t h);

    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    Display();
    void init(void);
    
    // ============================================
    // ÉCRAN DE DÉMARRAGE
    // ============================================
    void showSplashScreen(void);
    
    // ============================================
    // CONTRÔLE DE L'ÉCRAN
    // ============================================
    void powerOn(void);
    void powerOff(void);
    void setBrightness(uint8_t level);
    void clear(void);
    void refresh(void);
    
    // ============================================
    // BARRE D'ÉTAT (haute, 8px)
    // ============================================
    void updateSignal(int8_t rssi);
    void updateBattery(uint8_t percent, bool charging);
    void updateTime(uint8_t hour, uint8_t minute);
    void showMissedCallIcon(bool show);
    void showNewMessageIcon(bool show);
    void showSilentIcon(bool show);
    void showTorchIcon(bool show);
    
    // ============================================
    // ÉCRAN D'ACCUEIL
    // ============================================
    void showHomeScreen(void);
    void showHomeMenu(const char* items[], uint8_t count, uint8_t selected);
    
    // ============================================
    // ÉCRAN COMPOSEUR
    // ============================================
    void showDialerScreen(const char* number);
    void showDialerSuggestions(const char* suggestions[], uint8_t count);
    
    // ============================================
    // ÉCRAN APPEL
    // ============================================
    void showOutgoingCall(const char* name, const char* number);
    void showIncomingCall(const char* name, const char* number);
    void showCallActive(const char* name, uint32_t duration_sec, bool muted);
    void showCallEnded(const char* name, uint32_t duration_sec);
    void updateCallDuration(uint32_t duration_sec);
    
    // ============================================
    // ÉCRAN MESSAGES
    // ============================================
    void showMessageEditor(const char* text, const char* mode);
    void showMessagePreview(char currentChar);
    void showConversation(const char* messages[], uint8_t count, uint8_t scroll);
    void showMessageList(const char* contacts[], const char* previews[], uint8_t count, uint8_t selected);
    void showMessageSent(bool success);
    
    // ============================================
    // ÉCRAN CONTACTS
    // ============================================
    void showContactsList(const char* names[], uint8_t count, uint8_t selected);
    void showContactDetails(const char* name, const char* number, bool favorite);
    void showContactEditor(const char* name, const char* number, uint8_t field);
    
    // ============================================
    // ÉCRAN RÉGLAGES
    // ============================================
    void showSettingsMenu(const char* items[], uint8_t count, uint8_t selected);
    void showSettingOption(const char* title, const char* value);
    void showSettingSlider(const char* title, uint8_t value, uint8_t max);
    void showPinInput(uint8_t digits);
    void showPinError(void);
    
    // ============================================
    // BOÎTES DE DIALOGUE
    // ============================================
    void showAlert(const char* title, const char* message);
    void showConfirm(const char* title, const char* message);
    void showProgress(const char* message, uint8_t percent);
    void showLowBattery(uint8_t percent);
    
    // ============================================
    // UTILITAIRES
    // ============================================
    void drawText(uint8_t x, uint8_t y, const char* text, uint8_t size);
    void drawInvertedText(uint8_t x, uint8_t y, const char* text, uint8_t size);
    void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
    void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool filled);
    void drawProgressBar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percent);
    void drawStatusBar(void);
    ScreenType_t getCurrentScreen(void);
    void setScreen(ScreenType_t screen);
    bool isScreenOn(void);
};

#endif // DISPLAY_H