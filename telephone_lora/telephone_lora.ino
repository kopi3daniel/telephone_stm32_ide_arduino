/**
 * ---------------------------------------------------------------------------
 * teleogone_lora.ino - Point d entree principal du Telephone LoRa
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Ce fichier est le cerveau du telephone. Il :
 * - Initialise tous les modules
 * - Execute la boucle principale (machine d etats)
 * - Gere les interruptions (LoRa DIO0, boutons)
 */

// ============================================
// INCLUDES DU PROJET
// ============================================
#include "config.h"
#include "keyboard.h"
#include "display.h"
#include "lora_driver.h"
#include "audio.h"
#include "power_manager.h"
#include "call_manager.h"
#include "sms_manager.h"
#include "routing.h"
#include "crypto.h"
#include "home_screen.h"
#include "dialer_screen.h"
#include "call_screen.h"
#include "contacts_screen.h"
#include "sms_screen.h"
#include "settings_screen.h"
#include "status_bar.h"
#include "menu.h"
#include "dialog.h"
#include "input_field.h"
#include "icons.h"
#include "timers.h"
#include "compression.h"
#include "storage.h"
#include "crc.h"
#include "buffers.h"

// ============================================
// OBJETS GLOBAUX
// ============================================

// Drivers materiels
Keyboard keyboard;
Display display;
Audio audio;
LoRaDriver lora;
PowerManager powerManager;

// Protocoles
CallManager callManager;
SMSManager smsManager;
RoutingManager routing;
CRCChecksum crcChecksum;  // Renomme pour eviter conflit

// Ecrans
HomeScreen homeScreen;
DialerScreen dialerScreen;
CallScreen callScreen;
ContactsScreen contactsScreen;
SMSScreen smsScreen;
SettingsScreen settingsScreen;

// Widgets
StatusBar statusBar;
Menu menu;
Dialog dialog;
InputField inputField;

// Utilitaires
TimerManager timers;
Compression compression;
Storage storage;
EventQueue eventQueue;

// ============================================
// VARIABLES GLOBALES
// ============================================

typedef enum {
    APP_STATE_HOME,
    APP_STATE_DIALER,
    APP_STATE_IN_CALL,
    APP_STATE_CONTACTS,
    APP_STATE_MESSAGES,
    APP_STATE_SETTINGS,
    APP_STATE_LOCKED,
    APP_STATE_POWER_OFF
} AppState_t;

AppState_t appState = APP_STATE_HOME;
AppState_t previousState = APP_STATE_HOME;

uint32_t lastSecondTick = 0;
uint8_t seconds = 0;
uint8_t minutes = 0;
uint8_t hours = 12;

bool systemReady = false;
bool needRedraw = true;
uint32_t lastLoopTime = 0;

// ============================================
// PROTOTYPES
// ============================================
void processKeyPress(char key);
void updateSystemClock(void);
void goToState(AppState_t newState);
void goHome(void);
void onLoRaDIO0(void);

// ============================================
// SETUP - Initialisation Arduino
// ============================================
void setup() {
    
    // Port serie debug
    DEBUG_SERIAL.begin(DEBUG_BAUDRATE);
    DEBUG_SERIAL.println("\n\n--- Telephone LoRa - Demarrage ---");
    
    // Initialiser les modules
    DEBUG_SERIAL.println("Init Display...");
    display.init();
    
    DEBUG_SERIAL.println("Init Keyboard...");
    keyboard.init();
    
    DEBUG_SERIAL.println("Init Audio...");
    audio.init();
    
    DEBUG_SERIAL.println("Init LoRa...");
    lora.init();
    
    DEBUG_SERIAL.println("Init Power Manager...");
    powerManager.init();
    
    DEBUG_SERIAL.println("Init Call Manager...");
    callManager.init();
    
    DEBUG_SERIAL.println("Init SMS Manager...");
    smsManager.init();
    
    DEBUG_SERIAL.println("Init Routing...");
    routing.init();
    
    // Init ecrans
    homeScreen.init();
    dialerScreen.init();
    callScreen.init();
    contactsScreen.init();
    smsScreen.init();
    settingsScreen.init();
    
    // Init widgets
    statusBar.init();
    menu.init();
    inputField.init();
    
    // Configuration initiale
    statusBar.setSignal(-85);
    statusBar.setBattery(powerManager.getBatteryPercent(), powerManager.isCharging());
    statusBar.setTime(hours, minutes);
    
    // Activer la reception LoRa
    lora.setProfile(LORA_PROFILE_DATA);
    lora.startReceive();
    
    // Attacher l interruption DIO0
    attachInterrupt(digitalPinToInterrupt(LORA_DIO0_PIN), onLoRaDIO0, RISING);
    
    // Ecran d accueil
    goToState(APP_STATE_HOME);
    
    systemReady = true;
    lastLoopTime = millis();
    
    DEBUG_SERIAL.println("--- Telephone LoRa pret ! ---");
}

// ============================================
// BOUCLE PRINCIPALE
// ============================================
void loop() {
    
    uint32_t now = millis();
    
    // 1. Mise a jour des timers
    timers.update();
    
    // 2. Mise a jour de l horloge
    updateSystemClock();
    
    // 3. Gestion de l energie
    powerManager.process();
    
    // 4. Scanner le clavier
    char key = keyboard.scanKey();
    
    if (keyboard.isCallPressed()) key = 'A';
    if (keyboard.isEndPressed()) key = 'B';
    
    keyboard.checkMultiTapTimeout();
    
    // 5. Traiter la touche
    if (key != 0) {
        powerManager.userActivity();
        processKeyPress(key);
        needRedraw = true;
    }
    
    // 6. Verifier les paquets LoRa
    if (lora.isPacketAvailable()) {
        LoRaPacket_t packet;
        if (lora.receivePacket(&packet)) {
            
            switch (packet.packetType) {
                case PKT_CALL_REQUEST:
                case PKT_CALL_RINGING:
                case PKT_CALL_ACCEPT:
                case PKT_CALL_REJECT:
                case PKT_CALL_END:
                    callManager.handleCallPacket(packet.packetType, 
                                                 packet.senderId,
                                                 packet.payload, 
                                                 packet.payloadSize);
                    break;
                
                case PKT_VOICE_DATA:
                    callManager.handleVoicePacket(packet.payload, packet.payloadSize);
                    break;
                
                case PKT_SMS_TEXT:
                    smsManager.handleSMSPacket(packet.senderId, 
                                               packet.payload, 
                                               packet.payloadSize);
                    break;
                
                case PKT_PING:
                    routing.handlePing(packet.senderId);
                    break;
                
                case PKT_PONG:
                    routing.handlePong(packet.senderId, lora.getRSSI());
                    break;
                
                case PKT_PRESENCE:
                    routing.handlePresence(packet.senderId, 
                                          packet.payload, 
                                          packet.payloadSize);
                    break;
            }
            
            statusBar.setSignal(lora.getRSSI());
        }
        needRedraw = true;
    }
    
    // 7. Mettre a jour le module actif
    switch (appState) {
        case APP_STATE_HOME:     homeScreen.update(); break;
        case APP_STATE_DIALER:   dialerScreen.update(); break;
        case APP_STATE_IN_CALL:  callScreen.update(); callManager.process(); break;
        case APP_STATE_CONTACTS: contactsScreen.update(); break;
        case APP_STATE_MESSAGES: smsScreen.update(); break;
        case APP_STATE_SETTINGS: settingsScreen.update(); break;
        default: break;
    }
    
    // 8. Barre d etat
    statusBar.update();
    
    // 9. Dialogue
    if (dialog.isDialogVisible()) dialog.update();
    
    // 10. Routage
    routing.process();
    
    // 11. Sauvegarde periodique
    static uint32_t lastSave = 0;
    if (now - lastSave > 60000) {
        storage.saveAll();
        lastSave = now;
    }
    
    // 12. Redessiner
    if (needRedraw) {
        display.clear();
        
        switch (appState) {
            case APP_STATE_HOME:     homeScreen.draw(); break;
            case APP_STATE_DIALER:   dialerScreen.draw(); break;
            case APP_STATE_IN_CALL:  callScreen.draw(); break;
            case APP_STATE_CONTACTS: contactsScreen.draw(); break;
            case APP_STATE_MESSAGES: smsScreen.draw(); break;
            case APP_STATE_SETTINGS: settingsScreen.draw(); break;
            default: break;
        }
        
        if (dialog.isDialogVisible()) dialog.draw();
        
        display.refresh();
        needRedraw = false;
    }
    
    // 13. Petite pause
    delay(10);
    lastLoopTime = now;
}

// ============================================
// TRAITEMENT DES TOUCHES (version simplifiee)
// ============================================
void processKeyPress(char key) {
    
    if (dialog.isDialogVisible()) {
        dialog.handleKeyPress(key);
        return;
    }
    
    switch (appState) {
        case APP_STATE_HOME: {
            uint8_t action = homeScreen.handleKeyPress(key);
            if (action == 1) goToState(APP_STATE_DIALER);
            else if (action == 2) goToState(APP_STATE_MESSAGES);
            else if (action == 3) goToState(APP_STATE_CONTACTS);
            else if (action == 4) goToState(APP_STATE_SETTINGS);
            else if (action == 100) goToState(APP_STATE_DIALER);
            else if (action == 201) digitalToggle(TORCH_PIN);
            break;
        }
        
        case APP_STATE_DIALER: {
            uint8_t action = dialerScreen.handleKeyPress(key);
            if (action == 1) {
                const char* number = dialerScreen.getNumber();
                if (number && strlen(number) > 0) {
                    callManager.startCall(number);
                    callScreen.showOutgoingCall(number, number);
                    goToState(APP_STATE_IN_CALL);
                }
            } else if (action == 255) goHome();
            break;
        }
        
        case APP_STATE_IN_CALL: {
            uint8_t action = callScreen.handleKeyPress(key);
            if (action == CALL_OPTION_END) { callManager.endCall(); goHome(); }
            else if (action == CALL_OPTION_MUTE) callManager.toggleMute();
            else if (action == CALL_OPTION_SPEAKER) callManager.toggleSpeaker();
            else if (action == 255) goHome();
            break;
        }
        
        case APP_STATE_CONTACTS: {
            uint8_t action = contactsScreen.handleKeyPress(key);
            if (action == 1) {
                const char* number = contactsScreen.getSelectedNumber();
                if (number && strlen(number) > 0) {
                    callManager.startCall(number);
                    callScreen.showOutgoingCall(number, number);
                    goToState(APP_STATE_IN_CALL);
                }
            } else if (action == 255) goHome();
            break;
        }
        
        case APP_STATE_MESSAGES:
            if (smsScreen.handleKeyPress(key) == 255) goHome();
            break;
        
        case APP_STATE_SETTINGS:
            if (settingsScreen.handleKeyPress(key) == 255) goHome();
            break;
        
        default: break;
    }
}

// ============================================
// TRANSITIONS D ETAT
// ============================================
void goToState(AppState_t newState) {
    previousState = appState;
    appState = newState;
    
    switch (newState) {
        case APP_STATE_HOME:     homeScreen.onEnter(); break;
        case APP_STATE_DIALER:   dialerScreen.onEnter(); break;
        case APP_STATE_IN_CALL:  callScreen.onEnter(); break;
        case APP_STATE_CONTACTS: contactsScreen.onEnter(); break;
        case APP_STATE_MESSAGES: smsScreen.onEnter(); break;
        case APP_STATE_SETTINGS: settingsScreen.onEnter(); break;
        default: break;
    }
    needRedraw = true;
}

void goHome(void) { goToState(APP_STATE_HOME); }

// ============================================
// HORLOGE
// ============================================
void updateSystemClock(void) {
    uint32_t now = millis();
    
    if (now - lastSecondTick >= 1000) {
        seconds++;
        lastSecondTick = now;
        
        if (seconds >= 60) { seconds = 0; minutes++;
            if (minutes >= 60) { minutes = 0; hours++;
                if (hours >= 24) hours = 0;
            }
        }
        
        statusBar.setTime(hours, minutes);
        needRedraw = true;
    }
}

// ============================================
// INTERRUPTION LORA DIO0
// ============================================
void onLoRaDIO0(void) {
    lora.handleInterrupt();
}

// ============================================
// FIN DU FICHIER telegone_lora.ino
// ============================================