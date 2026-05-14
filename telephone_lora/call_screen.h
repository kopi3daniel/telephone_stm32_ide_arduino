/**
 * ---------------------------------------------------------------------------
 * call_screen.h - Ecran d appel en cours du Telephone LoRa
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Cet ecran gere l affichage pendant un appel telephonique.
 * Il montre :
 * - Le nom et numero du correspondant
 * - La duree de l appel en temps reel
 * - L etat de l appel (sonnerie, connecte, termine)
 * - Les options : muet, haut-parleur, raccrocher
 * - La qualite du signal pendant l appel
 */

#ifndef CALL_SCREEN_H
#define CALL_SCREEN_H

#include <Arduino.h>
#include "config.h"

// ============================================
// TYPES D ECRAN D APPEL
// ============================================
typedef enum {
    CALL_SCREEN_OUTGOING,       // Appel sortant (sonnerie)
    CALL_SCREEN_INCOMING,       // Appel entrant (sonnerie)
    CALL_SCREEN_ACTIVE,         // Communication en cours
    CALL_SCREEN_ENDED,          // Appel termine
    CALL_SCREEN_REJECTED,       // Appel refuse
    CALL_SCREEN_MISSED,         // Appel manque
    CALL_SCREEN_BUSY            // Correspondant occupe
} CallScreenType_t;

// ============================================
// OPTIONS PENDANT L APPEL
// ============================================
typedef enum {
    CALL_OPTION_MUTE,           // Couper/activer le micro
    CALL_OPTION_SPEAKER,        // Haut-parleur
    CALL_OPTION_HOLD,           // Mettre en attente
    CALL_OPTION_END,            // Raccrocher
    CALL_OPTION_ANSWER,         // Decrocher (appel entrant)
    CALL_OPTION_REJECT,         // Refuser (appel entrant)
    CALL_OPTION_COUNT
} CallOption_t;

// ============================================
// CLASSE CALL SCREEN
// ============================================
class CallScreen {
    
private:
    // ============================================
    // ETAT DE L ECRAN
    // ============================================
    CallScreenType_t screenType;    // Type d ecran affiche
    bool isActive;                  // Ecran actif ?
    uint32_t lastUpdate;            // Derniere mise a jour
    
    // ============================================
    // INFORMATIONS DE L APPEL
    // ============================================
    char callerName[32];            // Nom du correspondant
    char callerNumber[16];          // Numero du correspondant
    uint32_t callStartTime;         // Debut de l appel (pour duree)
    uint32_t callDuration;          // Duree en secondes
    bool isMuted;                   // Micro coupe ?
    bool isSpeakerOn;               // Haut-parleur actif ?
    int8_t signalQuality;           // Qualite du signal (0-4)
    
    // ============================================
    // ETAT DE LA SONNERIE (appel entrant)
    // ============================================
    uint32_t ringStartTime;         // Debut de la sonnerie
    bool ringToggled;               // Alternance sonnerie (clignotement)
    
    // ============================================
    // ANIMATIONS
    // ============================================
    uint8_t animationFrame;         // Frame d animation en cours
    uint32_t lastAnimationUpdate;   // Derniere mise a jour animation
    bool dotsVisible;               // Points de connexion visibles ?
    
    // ============================================
    // POSITIONS DES BOUTONS D OPTION
    // ============================================
    static const uint8_t BUTTON_Y = 48;     // Position Y des boutons
    static const uint8_t BUTTON_HEIGHT = 14; // Hauteur des boutons
    
    struct OptionButton {
        uint8_t x;
        uint8_t w;
        const char* label;
        bool selected;
    };
    
    OptionButton optionButtons[4];   // 4 boutons d option
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Dessine l ecran selon le type
    void drawOutgoingCall(void);
    void drawIncomingCall(void);
    void drawActiveCall(void);
    void drawEndedCall(void);
    
    // Dessine l en-tete (nom, numero)
    void drawCallerInfo(void);
    
    // Dessine la duree
    void drawDuration(void);
    
    // Dessine l icone d etat (telephone, horloge...)
    void drawStatusIcon(void);
    
    // Dessine les boutons d option
    void drawOptionButtons(void);
    
    // Dessine la barre de qualite du signal
    void drawSignalQuality(void);
    
    // Formate la duree en MM:SS
    void formatDuration(char* buffer, uint32_t seconds);
    
    // Animation de connexion (points qui defilent)
    void drawConnectingAnimation(void);
    
    // Icone telephone decroche (8x8)
    static const uint8_t icon_phone_active[8];
    
    // Icone telephone raccroche
    static const uint8_t icon_phone_idle[8];
    
    // Icone micro coupe
    static const uint8_t icon_mute[8];
    
    // Icone haut-parleur
    static const uint8_t icon_speaker[8];
    
    // Icone horloge (duree)
    static const uint8_t icon_clock[8];
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    CallScreen();
    void init(void);
    
    // ============================================
    // CYCLE DE VIE
    // ============================================
    void onEnter(void);
    void onExit(void);
    void update(void);
    void draw(void);
    
    // ============================================
    // CONFIGURATION DE L AFFICHAGE
    // ============================================
    
    // Affiche un appel sortant
    void showOutgoingCall(const char* name, const char* number);
    
    // Affiche un appel entrant
    void showIncomingCall(const char* name, const char* number);
    
    // Affiche l appel actif
    void showActiveCall(const char* name, const char* number);
    
    // Affiche la fin d appel
    void showCallEnded(const char* name, uint32_t duration);
    
    // Affiche un appel refuse
    void showCallRejected(const char* name);
    
    // Affiche un appel manque
    void showCallMissed(const char* name, const char* number);
    
    // Affiche "occupe"
    void showCallBusy(const char* name);
    
    // ============================================
    // MISE A JOUR EN TEMPS REEL
    // ============================================
    
    // Met a jour la duree
    void updateDuration(uint32_t duration);
    
    // Bascule l etat muet
    void toggleMute(void);
    
    // Bascule le haut-parleur
    void toggleSpeaker(void);
    
    // Met a jour la qualite du signal
    void updateSignalQuality(int8_t quality);
    
    // ============================================
    // GESTION DES TOUCHES
    // ============================================
    
    // Traite une touche
    // Retourne : CALL_OPTION_xxx
    uint8_t handleKeyPress(char key);
    
    // ============================================
    // INFORMATIONS
    // ============================================
    bool isActiveScreen(void);
    CallScreenType_t getType(void);
    bool isMuteActive(void);
    bool isSpeakerActive(void);
};

#endif // CALL_SCREEN_H