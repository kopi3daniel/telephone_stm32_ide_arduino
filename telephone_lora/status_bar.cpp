/**
 * ---------------------------------------------------------------------------
 * status_bar.cpp - Implementation de la barre d etat
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "status_bar.h"
#include "display.h"
#include <string.h>
#include <stdio.h>

extern Display display;

// ============================================
// BITMAPS DES ICONES (8x8 pixels)
// ============================================

// Signal - 0 barre
const uint8_t StatusBar::icon_signal_0[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E
};

// Signal - 1 barre
const uint8_t StatusBar::icon_signal_1[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x7E
};

// Signal - 2 barres
const uint8_t StatusBar::icon_signal_2[8] = {
    0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x7E
};

// Signal - 3 barres
const uint8_t StatusBar::icon_signal_3[8] = {
    0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E
};

// Signal - 4 barres
const uint8_t StatusBar::icon_signal_4[8] = {
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E
};

// Batterie pleine
const uint8_t StatusBar::icon_battery_full[8] = {
    0x7E, 0x42, 0x5A, 0x5A, 0x5A, 0x5A, 0x42, 0x7E
};

// Batterie 75%
const uint8_t StatusBar::icon_battery_75[8] = {
    0x7E, 0x42, 0x5A, 0x5A, 0x5A, 0x42, 0x42, 0x7E
};

// Batterie 50%
const uint8_t StatusBar::icon_battery_50[8] = {
    0x7E, 0x42, 0x5A, 0x5A, 0x42, 0x42, 0x42, 0x7E
};

// Batterie 25%
const uint8_t StatusBar::icon_battery_25[8] = {
    0x7E, 0x42, 0x5A, 0x42, 0x42, 0x42, 0x42, 0x7E
};

// Batterie vide
const uint8_t StatusBar::icon_battery_empty[8] = {
    0x7E, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7E
};

// Batterie en charge
const uint8_t StatusBar::icon_battery_charge[8] = {
    0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x3E, 0x1C, 0x08
};

// Icone message
const uint8_t StatusBar::icon_message[8] = {
    0x00, 0x7E, 0x42, 0x5A, 0x42, 0x24, 0x18, 0x00
};

// Icone appel manque
const uint8_t StatusBar::icon_call_missed[8] = {
    0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00
};

// Icone silencieux
const uint8_t StatusBar::icon_silent[8] = {
    0x00, 0x10, 0x18, 0x3C, 0x3C, 0x18, 0x10, 0x00
};

// Icone torche
const uint8_t StatusBar::icon_torch[8] = {
    0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x00
};

// Icone cadenas
const uint8_t StatusBar::icon_lock[8] = {
    0x00, 0x3C, 0x42, 0x42, 0x7E, 0x7E, 0x7E, 0x00
};

// Icone relais
const uint8_t StatusBar::icon_relay[8] = {
    0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x3E, 0x1C, 0x08
};

// ============================================
// CONSTRUCTEUR
// ============================================
StatusBar::StatusBar() {
    signalRSSI = -120;
    signalBars = 0;
    currentHour = 0;
    currentMinute = 0;
    clockVisible = true;
    batteryPercent = 100;
    batteryCharging = false;
    batteryLow = false;
    
    hasNewMessage = false;
    hasMissedCall = false;
    silentMode = false;
    torchOn = false;
    isLocked = false;
    relayMode = false;
    networkConnected = false;
    
    needsRedraw = true;
    lastUpdate = 0;
    lastBlink = 0;
    blinkState = false;
}

void StatusBar::init(void) {
    needsRedraw = true;
    lastUpdate = millis();
}

// ============================================
// MISE A JOUR PERIODIQUE
// ============================================
void StatusBar::update(void) {
    uint32_t now = millis();
    
    // Mettre a jour l heure toutes les 60 secondes
    if (now - lastUpdate > 60000) {
        currentMinute++;
        if (currentMinute >= 60) {
            currentMinute = 0;
            currentHour++;
            if (currentHour >= 24) {
                currentHour = 0;
            }
        }
        needsRedraw = true;
        lastUpdate = now;
    }
    
    // Faire clignoter les icones de notification toutes les 500ms
    if (now - lastBlink > 500) {
        blinkState = !blinkState;
        if (hasNewMessage || hasMissedCall || batteryLow) {
            needsRedraw = true;
        }
        lastBlink = now;
    }
    
    // Redessiner si necessaire
    if (needsRedraw) {
        draw();
        needsRedraw = false;
    }
}

void StatusBar::forceRedraw(void) {
    needsRedraw = true;
}

// ============================================
// CONVERSION RSSI -> BARRES
// ============================================
uint8_t StatusBar::rssiToBars(int8_t rssi) {
    if (rssi > -70)  return 4;
    if (rssi > -85)  return 3;
    if (rssi > -100) return 2;
    if (rssi > -115) return 1;
    return 0;
}

// ============================================
// SELECTION DES ICONES
// ============================================
const uint8_t* StatusBar::getSignalIcon(void) {
    switch (signalBars) {
        case 4: return icon_signal_4;
        case 3: return icon_signal_3;
        case 2: return icon_signal_2;
        case 1: return icon_signal_1;
        default: return icon_signal_0;
    }
}

const uint8_t* StatusBar::getBatteryIcon(void) {
    if (batteryCharging) return icon_battery_charge;
    if (batteryPercent > 75) return icon_battery_full;
    if (batteryPercent > 50) return icon_battery_75;
    if (batteryPercent > 25) return icon_battery_50;
    if (batteryPercent > 5)  return icon_battery_25;
    return icon_battery_empty;
}

// ============================================
// SETTERS
// ============================================
void StatusBar::setSignal(int8_t rssi) {
    signalRSSI = rssi;
    signalBars = rssiToBars(rssi);
    needsRedraw = true;
}

void StatusBar::setTime(uint8_t hour, uint8_t minute) {
    currentHour = hour;
    currentMinute = minute;
    needsRedraw = true;
}

void StatusBar::setBattery(uint8_t percent, bool charging) {
    batteryPercent = percent;
    batteryCharging = charging;
    batteryLow = (percent <= 15);
    needsRedraw = true;
}

void StatusBar::setNewMessage(bool has) { hasNewMessage = has; needsRedraw = true; }
void StatusBar::setMissedCall(bool has) { hasMissedCall = has; needsRedraw = true; }
void StatusBar::setSilentMode(bool silent) { silentMode = silent; needsRedraw = true; }
void StatusBar::setTorchOn(bool on) { torchOn = on; needsRedraw = true; }
void StatusBar::setLocked(bool locked) { isLocked = locked; needsRedraw = true; }
void StatusBar::setRelayMode(bool relay) { relayMode = relay; needsRedraw = true; }
void StatusBar::setNetworkConnected(bool connected) { networkConnected = connected; needsRedraw = true; }

// ============================================
// GETTERS
// ============================================
int8_t StatusBar::getSignalRSSI(void) { return signalRSSI; }
uint8_t StatusBar::getBatteryPercent(void) { return batteryPercent; }
bool StatusBar::hasNewMessageIndicator(void) { return hasNewMessage; }
bool StatusBar::hasMissedCallIndicator(void) { return hasMissedCall; }

// ============================================
// DESSIN DE LA BARRE D ETAT
// ============================================
void StatusBar::draw(void) {
    drawSignal();
    drawClock();
    drawBattery();
    drawNotifications();
}

void StatusBar::drawSignal(void) {
    const uint8_t* icon = getSignalIcon();
    display.drawIcon(SIGNAL_ICON_X, 0, icon, 8, 8);
    
    char rssiText[6];
    snprintf(rssiText, sizeof(rssiText), "%d", signalRSSI);
    display.drawText(SIGNAL_TEXT_X, 0, rssiText, 1);
}

void StatusBar::drawClock(void) {
    if (!clockVisible) return;
    
    char timeText[6];
    snprintf(timeText, sizeof(timeText), "%02d:%02d", currentHour, currentMinute);
    
    uint8_t textWidth = strlen(timeText) * 6;
    uint8_t x = (STATUSBAR_WIDTH - textWidth) / 2;
    display.drawText(x, 0, timeText, 1);
}

void StatusBar::drawBattery(void) {
    const uint8_t* icon = getBatteryIcon();
    display.drawIcon(BATTERY_ICON_X, 0, icon, 8, 8);
    
    char batText[5];
    snprintf(batText, sizeof(batText), "%d%%", batteryPercent);
    display.drawText(BATTERY_TEXT_X, 0, batText, 1);
    
    if (batteryLow && blinkState) {
        display.drawRect(BATTERY_ICON_X, 0, 22, 8, true);
        display.drawInvertedText(BATTERY_TEXT_X, 0, batText, 1);
    }
}

void StatusBar::drawNotifications(void) {
    uint8_t notifX = NOTIF_START_X;
    
    if (hasNewMessage) {
        if (blinkState) {
            display.drawIcon(notifX, 0, icon_message, 8, 8);
        }
        notifX += NOTIF_SPACING;
    }
    
    if (hasMissedCall) {
        if (blinkState) {
            display.drawIcon(notifX, 0, icon_call_missed, 8, 8);
        }
        notifX += NOTIF_SPACING;
    }
    
    if (silentMode) {
        display.drawIcon(notifX, 0, icon_silent, 8, 8);
        notifX += NOTIF_SPACING;
    }
    
    if (torchOn) {
        display.drawIcon(notifX, 0, icon_torch, 8, 8);
        notifX += NOTIF_SPACING;
    }
    
    if (isLocked) {
        display.drawIcon(notifX, 0, icon_lock, 8, 8);
        notifX += NOTIF_SPACING;
    }
    
    if (relayMode) {
        display.drawIcon(notifX, 0, icon_relay, 8, 8);
    }
}

// ============================================
// FIN DU FICHIER status_bar.cpp
// ============================================