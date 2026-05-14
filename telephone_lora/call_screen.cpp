/**
 * ---------------------------------------------------------------------------
 * call_screen.cpp - Implementation de l ecran d appel
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "call_screen.h"
#include "display.h"
#include <string.h>
#include <stdio.h>

extern Display display;

// ============================================
// ICONES BITMAP (8x8 pixels)
// ============================================

// Telephone decroche (appel actif)
const uint8_t CallScreen::icon_phone_active[8] = {
    0x00, 0x18, 0x3C, 0x3C, 0x3C, 0x18, 0x00, 0x00
};

// Telephone raccroche
const uint8_t CallScreen::icon_phone_idle[8] = {
    0x00, 0x00, 0x3C, 0x42, 0x42, 0x3C, 0x00, 0x00
};

// Micro coupe
const uint8_t CallScreen::icon_mute[8] = {
    0x00, 0x24, 0x18, 0x24, 0x00, 0x00, 0x00, 0x00
};

// Haut-parleur
const uint8_t CallScreen::icon_speaker[8] = {
    0x10, 0x18, 0x7E, 0x7E, 0x18, 0x10, 0x00, 0x00
};

// Horloge
const uint8_t CallScreen::icon_clock[8] = {
    0x3C, 0x42, 0x91, 0x91, 0x91, 0x42, 0x3C, 0x00
};

// ============================================
// CONSTRUCTEUR
// ============================================
CallScreen::CallScreen() {
    screenType = CALL_SCREEN_ACTIVE;
    isActive = false;
    lastUpdate = 0;
    
    memset(callerName, 0, sizeof(callerName));
    memset(callerNumber, 0, sizeof(callerNumber));
    callStartTime = 0;
    callDuration = 0;
    isMuted = false;
    isSpeakerOn = true;
    signalQuality = 3;
    
    ringStartTime = 0;
    ringToggled = false;
    
    animationFrame = 0;
    lastAnimationUpdate = 0;
    dotsVisible = true;
    
    // Configurer les boutons d option
    optionButtons[0].x = 0;
    optionButtons[0].w = 42;
    optionButtons[0].label = "Muet";
    optionButtons[0].selected = false;
    
    optionButtons[1].x = 43;
    optionButtons[1].w = 42;
    optionButtons[1].label = "HP";
    optionButtons[1].selected = false;
    
    optionButtons[2].x = 86;
    optionButtons[2].w = 42;
    optionButtons[2].label = "";
    optionButtons[2].selected = false;
    
    optionButtons[3].x = 0;
    optionButtons[3].w = 128;
    optionButtons[3].label = "Raccrocher";
    optionButtons[3].selected = false;
}

void CallScreen::init(void) {
    isActive = false;
    isMuted = false;
    isSpeakerOn = true;
}

// ============================================
// CYCLE DE VIE
// ============================================
void CallScreen::onEnter(void) {
    isActive = true;
    lastUpdate = millis();
    animationFrame = 0;
    draw();
}

void CallScreen::onExit(void) {
    isActive = false;
}

void CallScreen::update(void) {
    if (!isActive) return;
    
    uint32_t now = millis();
    
    // Mettre a jour l animation toutes les 500ms
    if (now - lastAnimationUpdate > 500) {
        dotsVisible = !dotsVisible;
        animationFrame++;
        lastAnimationUpdate = now;
        
        // Pour l appel entrant, alterner l affichage
        if (screenType == CALL_SCREEN_INCOMING) {
            ringToggled = !ringToggled;
        }
        
        draw();
    }
    
    // Mettre a jour la duree toutes les secondes
    if (screenType == CALL_SCREEN_ACTIVE && (now - lastUpdate > 1000)) {
        callDuration = (now - callStartTime) / 1000;
        lastUpdate = now;
        draw();
    }
}

// ============================================
// DESSIN PRINCIPAL
// ============================================
void CallScreen::draw(void) {
    display.clear();
    
    switch (screenType) {
        case CALL_SCREEN_OUTGOING:
            drawOutgoingCall();
            break;
        case CALL_SCREEN_INCOMING:
            drawIncomingCall();
            break;
        case CALL_SCREEN_ACTIVE:
            drawActiveCall();
            break;
        case CALL_SCREEN_ENDED:
        case CALL_SCREEN_REJECTED:
        case CALL_SCREEN_MISSED:
            drawEndedCall();
            break;
        default:
            break;
    }
    
    display.refresh();
}

// ============================================
// APPEL SORTANT (SONNERIE)
// ============================================
void CallScreen::drawOutgoingCall(void) {
    display.drawStatusBar();
    
    display.drawCenteredText(10, "Appel en cours...", 1);
    
    drawCallerInfo();
    drawConnectingAnimation();
    
    display.drawCenteredText(40, "Sonnerie...", 1);
    
    display.drawRect(10, BUTTON_Y, SCREEN_WIDTH - 20, BUTTON_HEIGHT, false);
    display.drawCenteredText(BUTTON_Y + 2, "Raccrocher", 1);
}

// ============================================
// APPEL ENTRANT (SONNERIE)
// ============================================
void CallScreen::drawIncomingCall(void) {
    display.drawStatusBar();
    
    if (ringToggled) {
        display.drawCenteredText(10, "APPEL ENTRANT", 1);
    }
    
    drawCallerInfo();
    
    if (ringToggled) {
        display.drawIcon(60, 20, icon_phone_active, 8, 8);
    }
    
    // Bouton VERT (decrocher) - gauche
    display.drawRect(2, BUTTON_Y, 60, BUTTON_HEIGHT, false);
    display.drawText(10, BUTTON_Y + 2, "Decrocher", 1);
    
    // Bouton ROUGE (refuser) - droite
    display.drawRect(66, BUTTON_Y, 60, BUTTON_HEIGHT, false);
    display.drawText(74, BUTTON_Y + 2, "Refuser", 1);
}

// ============================================
// APPEL ACTIF (COMMUNICATION)
// ============================================
void CallScreen::drawActiveCall(void) {
    display.drawStatusBar();
    
    display.drawText(2, 10, callerName, 1);
    display.drawText(2, 19, callerNumber, 1);
    
    display.drawLine(0, 28, SCREEN_WIDTH, 28);
    
    drawDuration();
    drawSignalQuality();
    drawOptionButtons();
}

// ============================================
// APPEL TERMINE
// ============================================
void CallScreen::drawEndedCall(void) {
    display.drawStatusBar();
    
    const char* title;
    switch (screenType) {
        case CALL_SCREEN_ENDED:    title = "Appel termine"; break;
        case CALL_SCREEN_REJECTED: title = "Appel refuse"; break;
        case CALL_SCREEN_MISSED:   title = "Appel manque"; break;
        default:                   title = "Fin d appel"; break;
    }
    
    display.drawCenteredText(10, title, 1);
    
    drawCallerInfo();
    
    if (callDuration > 0) {
        char durStr[20];
        formatDuration(durStr, callDuration);
        
        char label[30];
        snprintf(label, sizeof(label), "Duree: %s", durStr);
        display.drawCenteredText(42, label, 1);
    }
    
    display.drawCenteredText(SCREEN_HEIGHT - 10, "ROUGE: Retour", 1);
}

// ============================================
// SOUS-FONCTIONS DE DESSIN
// ============================================

void CallScreen::drawCallerInfo(void) {
    uint8_t y = 22;
    
    if (strlen(callerName) > 0) {
        display.drawText(2, y, callerName, 1);
        y += 10;
    }
    
    if (strlen(callerNumber) > 0) {
        display.drawText(2, y, callerNumber, 1);
    }
}

void CallScreen::drawDuration(void) {
    char durStr[10];
    formatDuration(durStr, callDuration);
    
    display.drawIcon(40, 32, icon_clock, 8, 8);
    display.drawText(50, 32, durStr, 2);
}

void CallScreen::drawSignalQuality(void) {
    uint8_t barX = 110;
    uint8_t barY = 34;
    
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t barH = 2 + (i * 2);
        
        if (i < (uint8_t)signalQuality) {
            display.drawRect(barX + (i * 4), barY + (8 - barH), 3, barH, true);
        } else {
            display.drawRect(barX + (i * 4), barY + (8 - barH), 3, barH, false);
        }
    }
}

void CallScreen::drawOptionButtons(void) {
    uint8_t btnY = 48;
    
    // Bouton Muet
    if (isMuted) {
        display.drawRect(2, btnY, 40, 14, true);
        display.drawInvertedText(6, btnY + 2, "Muet", 1);
    } else {
        display.drawRect(2, btnY, 40, 14, false);
        display.drawText(6, btnY + 2, "Muet", 1);
    }
    if (isMuted) {
        display.drawIcon(32, btnY + 2, icon_mute, 8, 8);
    }
    
    // Bouton Haut-parleur
    if (isSpeakerOn) {
        display.drawRect(46, btnY, 40, 14, true);
        display.drawInvertedText(50, btnY + 2, "HP", 1);
    } else {
        display.drawRect(46, btnY, 40, 14, false);
        display.drawText(50, btnY + 2, "HP", 1);
    }
    display.drawIcon(76, btnY + 2, icon_speaker, 8, 8);
    
    // Bouton Raccrocher
    display.drawRect(2, SCREEN_HEIGHT - 12, SCREEN_WIDTH - 4, 10, false);
    display.drawCenteredText(SCREEN_HEIGHT - 14, "ROUGE: Raccrocher", 1);
}

void CallScreen::drawConnectingAnimation(void) {
    uint8_t dotY = 35;
    uint8_t startX = 50;
    
    for (uint8_t i = 0; i < 3; i++) {
        if ((animationFrame + i) % 3 == 0) {
            display.drawRect(startX + (i * 12), dotY, 4, 4, true);
        } else {
            display.drawRect(startX + (i * 12), dotY, 4, 4, false);
        }
    }
}

// ============================================
// FORMATAGE DUREE
// ============================================
void CallScreen::formatDuration(char* buffer, uint32_t seconds) {
    uint32_t mins = seconds / 60;
    uint32_t secs = seconds % 60;
    snprintf(buffer, 10, "%02lu:%02lu", mins, secs);
}

// ============================================
// CONFIGURATION DE L AFFICHAGE
// ============================================
void CallScreen::showOutgoingCall(const char* name, const char* number) {
    screenType = CALL_SCREEN_OUTGOING;
    strncpy(callerName, name ? name : "Inconnu", 31);
    strncpy(callerNumber, number ? number : "", 15);
    callDuration = 0;
    callStartTime = millis();
    ringStartTime = millis();
    onEnter();
}

void CallScreen::showIncomingCall(const char* name, const char* number) {
    screenType = CALL_SCREEN_INCOMING;
    strncpy(callerName, name ? name : "Inconnu", 31);
    strncpy(callerNumber, number ? number : "", 15);
    callDuration = 0;
    ringStartTime = millis();
    onEnter();
}

void CallScreen::showActiveCall(const char* name, const char* number) {
    screenType = CALL_SCREEN_ACTIVE;
    strncpy(callerName, name ? name : "Inconnu", 31);
    strncpy(callerNumber, number ? number : "", 15);
    callStartTime = millis();
    callDuration = 0;
    onEnter();
}

void CallScreen::showCallEnded(const char* name, uint32_t duration) {
    screenType = CALL_SCREEN_ENDED;
    strncpy(callerName, name ? name : "Inconnu", 31);
    callDuration = duration;
    draw();
}

void CallScreen::showCallRejected(const char* name) {
    screenType = CALL_SCREEN_REJECTED;
    strncpy(callerName, name ? name : "Inconnu", 31);
    callDuration = 0;
    draw();
}

void CallScreen::showCallMissed(const char* name, const char* number) {
    screenType = CALL_SCREEN_MISSED;
    strncpy(callerName, name ? name : "Inconnu", 31);
    strncpy(callerNumber, number ? number : "", 15);
    callDuration = 0;
    draw();
}

void CallScreen::showCallBusy(const char* name) {
    screenType = CALL_SCREEN_BUSY;
    strncpy(callerName, name ? name : "Inconnu", 31);
    callDuration = 0;
    draw();
}

// ============================================
// MISE A JOUR EN TEMPS REEL
// ============================================
void CallScreen::updateDuration(uint32_t duration) {
    callDuration = duration;
}

void CallScreen::toggleMute(void) {
    isMuted = !isMuted;
    draw();
}

void CallScreen::toggleSpeaker(void) {
    isSpeakerOn = !isSpeakerOn;
    draw();
}

void CallScreen::updateSignalQuality(int8_t quality) {
    if (quality < 0) quality = 0;
    if (quality > 4) quality = 4;
    signalQuality = quality;
}

// ============================================
// GESTION DES TOUCHES
// ============================================
uint8_t CallScreen::handleKeyPress(char key) {
    if (!isActive) return 0;
    
    switch (screenType) {
        
        case CALL_SCREEN_INCOMING:
            switch (key) {
                case 'A':  // Bouton VERT = decrocher
                    return CALL_OPTION_ANSWER;
                case 'B':  // Bouton ROUGE = refuser
                    return CALL_OPTION_REJECT;
                default:
                    return 0;
            }
        
        case CALL_SCREEN_ACTIVE:
            switch (key) {
                case '*':  // Touche etoile = muet
                    toggleMute();
                    return CALL_OPTION_MUTE;
                case '0':  // Touche 0 = haut-parleur
                    toggleSpeaker();
                    return CALL_OPTION_SPEAKER;
                case 'B':  // Bouton ROUGE = raccrocher
                case '#':  // Touche diese = raccrocher aussi
                    return CALL_OPTION_END;
                default:
                    return 0;
            }
        
        case CALL_SCREEN_OUTGOING:
            if (key == 'B' || key == '#') {
                return CALL_OPTION_END;
            }
            return 0;
        
        case CALL_SCREEN_ENDED:
        case CALL_SCREEN_REJECTED:
        case CALL_SCREEN_MISSED:
            if (key == 'B' || key == '#') {
                return 255;  // Retour a l accueil
            }
            return 0;
        
        default:
            return 0;
    }
}

// ============================================
// INFORMATIONS
// ============================================
bool CallScreen::isActiveScreen(void) { return isActive; }
CallScreenType_t CallScreen::getType(void) { return screenType; }
bool CallScreen::isMuteActive(void) { return isMuted; }
bool CallScreen::isSpeakerActive(void) { return isSpeakerOn; }

// ============================================
// FIN DU FICHIER call_screen.cpp
// ============================================