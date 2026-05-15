

#include "home_screen.h"
#include "display.h"
#include <string.h>
#include <stdio.h>

// Reference a l ecran physique
extern Display display;

// ============================================
// BITMAPS 16x16 (2 octets par colonne, 16 colonnes)
// ============================================

// Icone Appels (combine telephonique)
// const uint8_t HomeScreen::icon_calls[16] = {
//     0x00, 0x00, 0x00, 0x00, 0x1F, 0xF8, 0x10, 0x08,
//     0x10, 0x08, 0x11, 0x88, 0x13, 0xC8, 0x11, 0x88,
//     0x10, 0x08, 0x10, 0x08, 0x1F, 0xF8, 0x00, 0x00,
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
// };

// // Icone Messages (bulle)
// const uint8_t HomeScreen::icon_messages[16] = {
//     0x00, 0x00, 0x0F, 0xF0, 0x10, 0x08, 0x20, 0x04,
//     0x20, 0x04, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08,
//     0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x18, 0x18,
//     0x0F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
// };

// // Icone Contacts (personne)
// const uint8_t HomeScreen::icon_contacts[16] = {
//     0x00, 0x00, 0x03, 0xC0, 0x04, 0x20, 0x04, 0x20,
//     0x04, 0x20, 0x03, 0xC0, 0x00, 0x00, 0x0F, 0xF0,
//     0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08,
//     0x1F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
// };

// // Icone Reglages (engrenage)
// const uint8_t HomeScreen::icon_settings[16] = {
//     0x00, 0x00, 0x06, 0x60, 0x06, 0x60, 0x0F, 0xF0,
//     0x19, 0x98, 0x18, 0x18, 0x0F, 0xF0, 0x06, 0x60,
//     0x0F, 0xF0, 0x18, 0x18, 0x19, 0x98, 0x0F, 0xF0,
//     0x06, 0x60, 0x06, 0x60, 0x00, 0x00, 0x00, 0x00
// };


// Icone Appels (combine telephonique) - 16x16 pixels = 32 octets
const uint8_t HomeScreen::icon_calls[32] = {
    0x00, 0x00, 0x00, 0x00, 0x1F, 0xF8, 0x10, 0x08,
    0x10, 0x08, 0x11, 0x88, 0x13, 0xC8, 0x11, 0x88,
    0x10, 0x08, 0x10, 0x08, 0x1F, 0xF8, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Icone Messages (bulle) - 16x16 pixels = 32 octets
const uint8_t HomeScreen::icon_messages[32] = {
    0x00, 0x00, 0x0F, 0xF0, 0x10, 0x08, 0x20, 0x04,
    0x20, 0x04, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08,
    0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x18, 0x18,
    0x0F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Icone Contacts (personne) - 16x16 pixels = 32 octets
const uint8_t HomeScreen::icon_contacts[32] = {
    0x00, 0x00, 0x03, 0xC0, 0x04, 0x20, 0x04, 0x20,
    0x04, 0x20, 0x03, 0xC0, 0x00, 0x00, 0x0F, 0xF0,
    0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08,
    0x1F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Icone Reglages (engrenage) - 16x16 pixels = 32 octets
const uint8_t HomeScreen::icon_settings[32] = {
    0x00, 0x00, 0x06, 0x60, 0x06, 0x60, 0x0F, 0xF0,
    0x19, 0x98, 0x18, 0x18, 0x0F, 0xF0, 0x06, 0x60,
    0x0F, 0xF0, 0x18, 0x18, 0x19, 0x98, 0x0F, 0xF0,
    0x06, 0x60, 0x06, 0x60, 0x00, 0x00, 0x00, 0x00
};



// Noms des jours
const char* HomeScreen::dayNames[7] = {
    "Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"
};

// ============================================
// CONSTRUCTEUR
// ============================================
HomeScreen::HomeScreen() {
    selectedItem = HOME_MENU_CALLS;
    isActive = false;
    lastUpdate = 0;
    
    currentHour = 0;
    currentMinute = 0;
    currentDay = 1;
    currentMonth = 1;
    strcpy(dayName, "---");
    
    signalRSSI = -120;
    batteryPercent = 100;
    batteryCharging = false;
    
    unreadMessages = 0;
    missedCalls = 0;
    silentMode = false;
    torchOn = false;
    
    // Positions des 4 icones (centrees horizontalement)
    uint8_t totalWidth = (ICON_SIZE * 4) + (ICON_SPACING * 3);
    uint8_t startX = (SCREEN_WIDTH - totalWidth) / 2;
    
    for (uint8_t i = 0; i < 4; i++) {
        iconPositions[i].x = startX + i * (ICON_SIZE + ICON_SPACING);
        iconPositions[i].y = 38;
    }
}

void HomeScreen::init(void) {
    selectedItem = HOME_MENU_CALLS;
    isActive = false;
}

// ============================================
// CYCLE DE VIE
// ============================================
void HomeScreen::onEnter(void) {
    isActive = true;
    lastUpdate = millis();
    draw();
}

void HomeScreen::onExit(void) {
    isActive = false;
}

void HomeScreen::update(void) {
    if (!isActive) return;
    
    // Mettre a jour toutes les 500ms (pour l horloge)
    uint32_t now = millis();
    if (now - lastUpdate > 500) {
        updateClock();
        draw();
        lastUpdate = now;
    }
}

// ============================================
// DESSIN PRINCIPAL
// ============================================
void HomeScreen::draw(void) {
    display.clear();
    
    // Barre d etat
    display.updateSignal(signalRSSI);
    display.updateBattery(batteryPercent, batteryCharging);
    display.updateTime(currentHour, currentMinute);
    
    // Dessiner l horloge
    drawClock();
    
    // Dessiner la date
    drawDate();
    
    // Ligne de separation
    display.drawLine(0, 36, SCREEN_WIDTH, 36);
    
    // Dessiner les icones du menu
    drawMenuIcons();
    
    // Dessiner les badges de notification
    drawNotificationBadges();
    
    display.refresh();
}

// ============================================
// HORLOGE
// ============================================
void HomeScreen::drawClock(void) {
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", currentHour, currentMinute);
    
    // Grande police centree
    display.drawCenteredText(12, timeStr, 3);
}

void HomeScreen::drawDate(void) {
    char dateStr[20];
    snprintf(dateStr, sizeof(dateStr), "%s %02d %s", 
             dayName, currentDay, getMonthName(currentMonth));
    
    display.drawCenteredText(30, dateStr, 1);
}

void HomeScreen::updateClock(void) {
    // Simulation : incrementer les minutes
    // En pratique, on lirait le RTC
    currentMinute++;
    if (currentMinute >= 60) {
        currentMinute = 0;
        currentHour++;
        if (currentHour >= 24) {
            currentHour = 0;
            currentDay++;
        }
    }
}

// ============================================
// ICONES DU MENU
// ============================================
void HomeScreen::drawMenuIcons(void) {
    const uint8_t* icons[] = {
        icon_calls,
        icon_messages,
        icon_contacts,
        icon_settings
    };
    
    const char* labels[] = {
        "Appels",
        "SMS",
        "Contacts",
        "Reglages"
    };
    
    for (uint8_t i = 0; i < 4; i++) {
        drawMenuIcon(i, icons[i], labels[i]);
    }
    
    // Surligner l element selectionne
    highlightSelected();
}

void HomeScreen::drawMenuIcon(uint8_t index, const uint8_t* bitmap, const char* label) {
    uint8_t x = iconPositions[index].x;
    uint8_t y = iconPositions[index].y;
    
    // Dessiner l icone
    display.drawIcon(x, y, bitmap, ICON_SIZE, ICON_SIZE);
    
    // Dessiner l etiquette
    uint8_t labelWidth = strlen(label) * 6;  // Police 6x8
    uint8_t labelX = x + (ICON_SIZE - labelWidth) / 2;
    display.drawText(labelX, y + ICON_SIZE + 2, label, 1);
}

void HomeScreen::highlightSelected(void) {
    uint8_t x = iconPositions[selectedItem].x;
    uint8_t y = iconPositions[selectedItem].y;
    
    // Rectangle autour de l icone selectionnee
    display.drawRect(x - 2, y - 2, ICON_SIZE + 4, ICON_SIZE + 14, false);
}

// ============================================
// BADGES DE NOTIFICATION
// ============================================
void HomeScreen::drawNotificationBadges(void) {
    // Badge messages non lus (sur l icone SMS)
    if (unreadMessages > 0) {
        drawBadge(iconPositions[HOME_MENU_MESSAGES].x + ICON_SIZE - 4,
                  iconPositions[HOME_MENU_MESSAGES].y - 2,
                  unreadMessages);
    }
    
    // Badge appels manques (sur l icone Appels)
    if (missedCalls > 0) {
        drawBadge(iconPositions[HOME_MENU_CALLS].x + ICON_SIZE - 4,
                  iconPositions[HOME_MENU_CALLS].y - 2,
                  missedCalls);
    }
}

void HomeScreen::drawBadge(uint8_t x, uint8_t y, uint8_t count) {
    // Cercle rouge (simule par un carre plein)
    display.drawRect(x, y, 8, 8, true);
    
    // Nombre en noir sur fond blanc
    char countStr[3];
    snprintf(countStr, sizeof(countStr), "%d", count > 9 ? 9 : count);
    display.drawInvertedText(x + 1, y, countStr, 1);
}

// ============================================
// GESTION DES TOUCHES
// ============================================
uint8_t HomeScreen::handleKeyPress(char key) {
    if (!isActive) return 0;
    
    switch (key) {
        case '2':  // Navigation Haut
            moveSelectionUp();
            return 0;
            
        case '8':  // Navigation Bas
            moveSelectionDown();
            return 0;
            
        case '4':  // Navigation Gauche
            moveSelectionLeft();
            return 0;
            
        case '6':  // Navigation Droite
            moveSelectionRight();
            return 0;
            
        case 'A':  // Bouton Appel = selectionner
        case '5':  // Centre = selectionner
            selectItem();
            return selectedItem + 1;  // Retourne l action (1-4)
            
        case '1': case '3': case '7': case '9':
        case '0':
            // Touche numerique = ouvrir composeur rapide
            return 100;
            
        case 'B':  // Bouton Retour = verrouiller
            return 200;
            
        case '*':  // Etoile = torche
            return 201;
            
        case '#':  // Diese = mode silencieux
            return 202;
            
        default:
            return 0;
    }
}

void HomeScreen::moveSelectionLeft(void) {
    if (selectedItem > 0) {
        selectedItem = (HomeMenuItem_t)(selectedItem - 1);
        draw();
    }
}

void HomeScreen::moveSelectionRight(void) {
    if (selectedItem < HOME_MENU_COUNT - 1) {
        selectedItem = (HomeMenuItem_t)(selectedItem + 1);
        draw();
    }
}

void HomeScreen::moveSelectionUp(void) {
    moveSelectionLeft();
}

void HomeScreen::moveSelectionDown(void) {
    moveSelectionRight();
}

void HomeScreen::selectItem(void) {
    // L action est retournee par handleKeyPress
}

// ============================================
// MISE A JOUR DES DONNEES
// ============================================
void HomeScreen::setSignal(int8_t rssi) {
    signalRSSI = rssi;
}

void HomeScreen::setBattery(uint8_t percent, bool charging) {
    batteryPercent = percent;
    batteryCharging = charging;
}

void HomeScreen::setTime(uint8_t hour, uint8_t minute) {
    currentHour = hour;
    currentMinute = minute;
}

// void HomeScreen::setDate(uint8_t day, uint8_t month, const char* day) {
//     currentDay = day;
//     currentMonth = month;
//     if (day != NULL) {
//         strncpy(dayName, day, 3);
//         dayName[3] = '\0';
//     }
// }

void HomeScreen::setDate(uint8_t day, uint8_t month, const char* dayStr) {
    currentDay = day;
    currentMonth = month;
    if (dayStr != NULL) {
        strncpy(dayName, dayStr, 3);
        dayName[3] = '\0';
    }
}

void HomeScreen::setUnreadMessages(uint8_t count) {
    unreadMessages = count;
    if (isActive) draw();
}

void HomeScreen::setMissedCalls(uint8_t count) {
    missedCalls = count;
    if (isActive) draw();
}

void HomeScreen::setSilentMode(bool silent) {
    silentMode = silent;
}

void HomeScreen::setTorchOn(bool on) {
    torchOn = on;
}

// ============================================
// INFORMATIONS
// ============================================
HomeMenuItem_t HomeScreen::getSelectedItem(void) {
    return selectedItem;
}

bool HomeScreen::isActiveScreen(void) {
    return isActive;
}

// ============================================
// UTILITAIRES
// ============================================
const char* HomeScreen::getMonthName(uint8_t month) {
    static const char* months[] = {
        "Jan", "Fev", "Mar", "Avr", "Mai", "Juin",
        "Juil", "Aout", "Sep", "Oct", "Nov", "Dec"
    };
    
    if (month >= 1 && month <= 12) {
        return months[month - 1];
    }
    return "???";
}

// ============================================
// FIN DU FICHIER home_screen.cpp
// ============================================