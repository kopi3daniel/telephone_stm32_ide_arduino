
 /* ---------------------------------------------------------------------------
 * settings_screen.cpp - Implementation de l ecran des parametres
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "settings_screen.h"
#include "display.h"
#include "keyboard.h"
#include <string.h>
#include <stdio.h>

extern Display display;
extern Keyboard keyboard;

// ============================================
// NOMS DES CATEGORIES
// ============================================
static const char* categoryNames[] = {
    "Son et vibrations",
    "Affichage",
    "Reseau LoRa",
    "Securite",
    "Energie",
    "Systeme"
};

// ============================================
// NOMS DES FREQUENCES
// ============================================
static const char* frequencyNames[] = {
    "868 MHz (Europe)",
    "915 MHz (Ameriques)",
    "433 MHz (Asie/Afrique)"
};

// ============================================
// NOMS DES SONNERIES
// ============================================
static const char* ringtoneNames[] = {
    "Classique",
    "Melodie 1",
    "Melodie 2",
    "Vibreur seul",
    "Bip court",
    "Bip long",
    "Sonnerie 1",
    "Sonnerie 2",
    "Silencieux",
    "Personnalise"
};

// ============================================
// CONSTRUCTEUR
// ============================================
SettingsScreen::SettingsScreen() {
    mode = SETTINGS_MODE_MAIN_MENU;
    category = SETTINGS_CATEGORY_SOUND;
    isActive = false;
    lastUpdate = 0;
    
    selectedItem = 0;
    itemCount = SETTINGS_CATEGORY_COUNT;
    scrollOffset = 0;
    
    memset(options, 0, sizeof(options));
    optionCount = 0;
    
    // Valeurs par defaut
    ringVolume = 7;
    ringTone = 0;
    vibrateEnabled = true;
    silentMode = false;
    
    brightness = 200;
    screenTimeout = 30;
    
    frequencyIndex = 0;
    txPower = 17;
    phoneId = PHONE_ID;
    
    strcpy(pinCode, "0000");
    lockEnabled = false;
    encryptionEnabled = AES_ENABLED;
    
    powerSavingEnabled = true;
    sleepTimeout = 5;
    
    memset(pinEntry, 0, sizeof(pinEntry));
    pinEntryIndex = 0;
    pinConfirmMode = false;
    memset(pinNew, 0, sizeof(pinNew));
}

void SettingsScreen::init(void) {
    isActive = false;
    mode = SETTINGS_MODE_MAIN_MENU;
    selectedItem = 0;
    loadSettings();
}

// ============================================
// CYCLE DE VIE
// ============================================
void SettingsScreen::onEnter(void) {
    isActive = true;
    loadSettings();
    draw();
}

void SettingsScreen::onExit(void) {
    isActive = false;
    saveSettings();
}

void SettingsScreen::update(void) {
    if (!isActive) return;
}

// ============================================
// CHARGEMENT/SAUVEGARDE
// ============================================
void SettingsScreen::loadSettings(void) {
    // TODO: Charger depuis le module storage
}

void SettingsScreen::saveSettings(void) {
    // TODO: Sauvegarder dans le module storage
}

// ============================================
// DESSIN PRINCIPAL
// ============================================
void SettingsScreen::draw(void) {
    display.clear();
    
    switch (mode) {
        case SETTINGS_MODE_MAIN_MENU:
            drawMainMenu();
            break;
        case SETTINGS_MODE_SUB_MENU:
            drawSubMenu();
            break;
        case SETTINGS_MODE_SLIDER:
            drawSlider();
            break;
        case SETTINGS_MODE_SELECT:
            drawSelectList();
            break;
        case SETTINGS_MODE_PIN_ENTRY:
            drawPinEntry();
            break;
        case SETTINGS_MODE_CONFIRM:
            drawConfirm();
            break;
    }
    
    display.refresh();
}

// ============================================
// MENU PRINCIPAL
// ============================================
void SettingsScreen::drawMainMenu(void) {
    display.drawCenteredText(0, "Reglages", 1);
    display.drawLine(0, 9, SCREEN_WIDTH, 9);
    
    itemCount = SETTINGS_CATEGORY_COUNT;
    
    if (selectedItem >= scrollOffset + 5) {
        scrollOffset = selectedItem - 4;
    }
    if (selectedItem < scrollOffset) {
        scrollOffset = selectedItem;
    }
    
    for (uint8_t i = 0; i < 5 && (scrollOffset + i) < itemCount; i++) {
        uint8_t idx = scrollOffset + i;
        uint8_t y = 12 + (i * 10);
        drawMenuItem(idx, y, (idx == selectedItem));
    }
    
    display.drawText(0, SCREEN_HEIGHT - 10, "VERT:Ouvrir", 1);
    display.drawText(SCREEN_WIDTH - 50, SCREEN_HEIGHT - 10, "B:Retour", 1);
}

// ============================================
// SOUS-MENU
// ============================================
void SettingsScreen::drawSubMenu(void) {
    display.drawCenteredText(0, categoryNames[category], 1);
    display.drawLine(0, 9, SCREEN_WIDTH, 9);
    
    for (uint8_t i = 0; i < optionCount && i < 5; i++) {
        uint8_t y = 12 + (i * 10);
        drawMenuItem(i, y, (i == selectedItem));
    }
    
    display.drawText(0, SCREEN_HEIGHT - 10, "VERT:Modifier", 1);
    display.drawText(SCREEN_WIDTH - 50, SCREEN_HEIGHT - 10, "B:Retour", 1);
}

// ============================================
// ELEMENT DE MENU
// ============================================
void SettingsScreen::drawMenuItem(uint8_t index, uint8_t y, bool selected) {
    char displayText[40];
    
    if (mode == SETTINGS_MODE_MAIN_MENU && index < SETTINGS_CATEGORY_COUNT) {
        strncpy(displayText, categoryNames[index], 35);
        displayText[35] = '\0';
    } else if (mode == SETTINGS_MODE_SUB_MENU && index < optionCount) {
        snprintf(displayText, sizeof(displayText), "%s", options[index].name);
    } else {
        return;
    }
    
    if (selected) {
        display.drawRect(0, y - 1, SCREEN_WIDTH, 10, true);
        display.drawInvertedText(2, y, displayText, 1);
    } else {
        display.drawText(2, y, displayText, 1);
    }
    
    if (mode == SETTINGS_MODE_SUB_MENU && index < optionCount) {
        char valueStr[10];
        snprintf(valueStr, sizeof(valueStr), "%d", options[index].value);
        uint8_t x = SCREEN_WIDTH - (strlen(valueStr) * 6) - 4;
        
        if (selected) {
            display.drawInvertedText(x, y, valueStr, 1);
        } else {
            display.drawText(x, y, valueStr, 1);
        }
    }
}

// ============================================
// CURSEUR
// ============================================
void SettingsScreen::drawSlider(void) {
    if (selectedItem >= optionCount) return;
    
    display.drawCenteredText(0, options[selectedItem].name, 1);
    display.drawLine(0, 9, SCREEN_WIDTH, 9);
    
    char valueStr[5];
    snprintf(valueStr, sizeof(valueStr), "%d", options[selectedItem].value);
    display.drawCenteredText(25, valueStr, 2);
    
    drawSliderBar(40, options[selectedItem].value, options[selectedItem].maxValue);
    
    display.drawText(2, SCREEN_HEIGHT - 10, "4/6:- +", 1);
    display.drawText(SCREEN_WIDTH - 60, SCREEN_HEIGHT - 10, "VERT:OK", 1);
    display.drawText(SCREEN_WIDTH - 25, SCREEN_HEIGHT - 10, "B:Retour", 1);
}

void SettingsScreen::drawSliderBar(uint8_t y, uint8_t value, uint8_t maxValue) {
    uint8_t barWidth = 100;
    uint8_t barX = (SCREEN_WIDTH - barWidth) / 2;
    
    display.drawRect(barX, y, barWidth, 8, false);
    
    if (maxValue > 0) {
        uint8_t filled = (uint16_t)(value * barWidth) / maxValue;
        if (filled > 0) {
            display.drawRect(barX, y, filled, 8, true);
        }
        
        uint8_t indicatorX = barX + filled - 2;
        if (indicatorX < barX) indicatorX = barX;
        if (indicatorX > barX + barWidth - 4) indicatorX = barX + barWidth - 4;
        display.drawRect(indicatorX, y - 1, 4, 10, true);
    }
}

// ============================================
// LISTE DE SELECTION
// ============================================
void SettingsScreen::drawSelectList(void) {
    if (selectedItem >= optionCount) return;
    
    display.drawCenteredText(0, options[selectedItem].name, 1);
    display.drawLine(0, 9, SCREEN_WIDTH, 9);
    
    if (options[selectedItem].choices != NULL) {
        for (uint8_t i = 0; i < options[selectedItem].choiceCount && i < 5; i++) {
            uint8_t y = 12 + (i * 10);
            bool isSel = (i == options[selectedItem].value);
            
            if (isSel) {
                display.drawRect(0, y - 1, SCREEN_WIDTH, 10, true);
                display.drawInvertedText(2, y, options[selectedItem].choices[i], 1);
                display.drawInvertedText(SCREEN_WIDTH - 12, y, "v", 1);
            } else {
                display.drawText(2, y, options[selectedItem].choices[i], 1);
            }
        }
    }
    
    display.drawText(0, SCREEN_HEIGHT - 10, "VERT:Selection", 1);
    display.drawText(SCREEN_WIDTH - 50, SCREEN_HEIGHT - 10, "B:Retour", 1);
}

// ============================================
// SAISIE CODE PIN
// ============================================
void SettingsScreen::drawPinEntry(void) {
    const char* title = pinConfirmMode ? "Nouveau PIN" : "Code PIN";
    display.drawCenteredText(0, title, 1);
    display.drawLine(0, 9, SCREEN_WIDTH, 9);
    
    if (pinConfirmMode) {
        display.drawCenteredText(18, "Confirmez le nouveau code", 1);
    }
    
    display.drawCenteredText(30, "____", 2);
    
    for (uint8_t i = 0; i < pinEntryIndex; i++) {
        display.drawText(48 + (i * 12), 30, "*", 2);
    }
    
    display.drawText(2, 50, "C:Effacer", 1);
    display.drawText(SCREEN_WIDTH - 50, 50, "B:Annuler", 1);
}

// ============================================
// CONFIRMATION
// ============================================
void SettingsScreen::drawConfirm(void) {
    display.drawCenteredText(0, "Confirmation", 1);
    display.drawLine(0, 9, SCREEN_WIDTH, 9);
    display.drawCenteredText(28, "Etes-vous sur ?", 1);
    display.drawText(15, 45, "VERT:Oui", 1);
    display.drawText(70, 45, "ROUGE:Non", 1);
}

// ============================================
// CONSTRUCTION DES MENUS PAR CATEGORIE
// ============================================
void SettingsScreen::buildCategoryMenu(SettingsCategory_t cat) {
    optionCount = 0;
    memset(options, 0, sizeof(options));
    
    switch (cat) {
        case SETTINGS_CATEGORY_SOUND:
            options[0] = (SettingOption_t){"Volume", ringVolume, 0, 10, NULL, 0};
            options[1] = (SettingOption_t){"Sonnerie", ringTone, 0, 9, ringtoneNames, 10};
            options[2] = (SettingOption_t){"Vibreur", vibrateEnabled, 0, 1, NULL, 0};
            options[3] = (SettingOption_t){"Mode silencieux", silentMode, 0, 1, NULL, 0};
            optionCount = 4;
            break;
            
        case SETTINGS_CATEGORY_DISPLAY:
            options[0] = (SettingOption_t){"Luminosite", brightness, 10, 255, NULL, 0};
            options[1] = (SettingOption_t){"Extinction ecran", screenTimeout, 10,255, NULL, 0};
            optionCount = 2;
            break;
            
        case SETTINGS_CATEGORY_NETWORK:
            options[0] = (SettingOption_t){"Frequence", frequencyIndex, 0, 2, frequencyNames, 3};
            options[1] = (SettingOption_t){"Puissance TX", txPower, 2, 20, NULL, 0};
            optionCount = 2;
            break;
            
        case SETTINGS_CATEGORY_SECURITY:
            options[0] = (SettingOption_t){"Code PIN", lockEnabled, 0, 1, NULL, 0};
            options[1] = (SettingOption_t){"Chiffrement", encryptionEnabled, 0, 1, NULL, 0};
            optionCount = 2;
            break;
            
        case SETTINGS_CATEGORY_POWER:
            options[0] = (SettingOption_t){"Mode economie", powerSavingEnabled, 0, 1, NULL, 0};
            options[1] = (SettingOption_t){"Veille (min)", sleepTimeout, 1, 60, NULL, 0};
            optionCount = 2;
            break;
            
        case SETTINGS_CATEGORY_SYSTEM:
            options[0] = (SettingOption_t){"Version", 0, 0, 0, NULL, 0};
            options[1] = (SettingOption_t){"Reset usine", 0, 0, 0, NULL, 0};
            optionCount = 2;
            break;
            
        default:
            break;
    }
}

// ============================================
// GESTION DES TOUCHES
// ============================================
uint8_t SettingsScreen::handleKeyPress(char key) {
    if (!isActive) return 0;
    
    switch (mode) {
        
        case SETTINGS_MODE_MAIN_MENU:
            switch (key) {
                case '2': if (selectedItem > 0) selectedItem--; draw(); return 0;
                case '8': if (selectedItem < itemCount - 1) selectedItem++; draw(); return 0;
                case 'A':
                    category = (SettingsCategory_t)selectedItem;
                    buildCategoryMenu(category);
                    selectedItem = 0;
                    mode = SETTINGS_MODE_SUB_MENU;
                    draw();
                    return 0;
                case 'B': return 255;
                default: return 0;
            }
        
        case SETTINGS_MODE_SUB_MENU:
            switch (key) {
                case '2': if (selectedItem > 0) selectedItem--; draw(); return 0;
                case '8': if (selectedItem < optionCount - 1) selectedItem++; draw(); return 0;
                case 'A':
                    if (options[selectedItem].choices != NULL) {
                        mode = SETTINGS_MODE_SELECT;
                    } else if (options[selectedItem].maxValue > 0) {
                        mode = SETTINGS_MODE_SLIDER;
                    } else if (category == SETTINGS_CATEGORY_SECURITY && selectedItem == 0) {
                        mode = SETTINGS_MODE_PIN_ENTRY;
                        pinEntryIndex = 0;
                        memset(pinEntry, 0, sizeof(pinEntry));
                    } else if (category == SETTINGS_CATEGORY_SYSTEM && selectedItem == 1) {
                        mode = SETTINGS_MODE_CONFIRM;
                    }
                    draw();
                    return 0;
                case 'B':
                    mode = SETTINGS_MODE_MAIN_MENU;
                    selectedItem = category;
                    draw();
                    return 0;
                default: return 0;
            }
        
        case SETTINGS_MODE_SLIDER:
            switch (key) {
                case '4':
                case '6':
                    if (key == '6' && options[selectedItem].value < options[selectedItem].maxValue)
                        options[selectedItem].value++;
                    if (key == '4' && options[selectedItem].value > options[selectedItem].minValue)
                        options[selectedItem].value--;
                    applySetting();
                    draw();
                    return 0;
                case 'A':
                    mode = SETTINGS_MODE_SUB_MENU;
                    saveSettings();
                    draw();
                    return 0;
                case 'B':
                    mode = SETTINGS_MODE_SUB_MENU;
                    draw();
                    return 0;
                default: return 0;
            }
        
        case SETTINGS_MODE_SELECT:
            switch (key) {
                case '2': 
                    if (options[selectedItem].value > 0) options[selectedItem].value--; 
                    draw(); return 0;
                case '8': 
                    if (options[selectedItem].value < options[selectedItem].choiceCount - 1) 
                        options[selectedItem].value++; 
                    draw(); return 0;
                case 'A': 
                    mode = SETTINGS_MODE_SUB_MENU; 
                    applySetting(); 
                    saveSettings(); 
                    draw(); 
                    return 0;
                case 'B': 
                    mode = SETTINGS_MODE_SUB_MENU; 
                    draw(); 
                    return 0;
                default: return 0;
            }
        
        case SETTINGS_MODE_PIN_ENTRY:
            if (key >= '0' && key <= '9') {
                if (pinEntryIndex < 4) {
                    pinEntry[pinEntryIndex++] = key;
                    pinEntry[pinEntryIndex] = '\0';
                    if (pinEntryIndex == 4) {
                        if (pinConfirmMode) {
                            if (strcmp(pinEntry, pinNew) == 0) {
                                strcpy(pinCode, pinNew);
                                lockEnabled = true;
                                mode = SETTINGS_MODE_SUB_MENU;
                                pinConfirmMode = false;
                            } else {
                                // Codes differents
                                pinEntryIndex = 0;
                                memset(pinEntry, 0, sizeof(pinEntry));
                            }
                        } else {
                            if (strcmp(pinEntry, pinCode) == 0) {
                                // PIN correct - demander nouveau PIN
                                pinConfirmMode = true;
                                memset(pinNew, 0, sizeof(pinNew));
                                pinEntryIndex = 0;
                                memset(pinEntry, 0, sizeof(pinEntry));
                            } else {
                                pinEntryIndex = 0;
                                memset(pinEntry, 0, sizeof(pinEntry));
                            }
                        }
                    }
                }
                draw(); return 0;
            }
            switch (key) {
                case 'C': 
                    if (pinEntryIndex > 0) pinEntry[--pinEntryIndex] = '\0'; 
                    draw(); return 0;
                case 'B': 
                    mode = SETTINGS_MODE_SUB_MENU; 
                    pinConfirmMode = false;
                    draw(); return 0;
                default: return 0;
            }
        
        case SETTINGS_MODE_CONFIRM:
            switch (key) {
                case 'A': 
                    mode = SETTINGS_MODE_MAIN_MENU; 
                    draw(); return 0;
                case 'B': 
                    mode = SETTINGS_MODE_MAIN_MENU; 
                    draw(); return 0;
                default: return 0;
            }
        
        default: return 0;
    }
}

// ============================================
// APPLIQUER UN REGLAGE
// ============================================
void SettingsScreen::applySetting(void) {
    // Mettre a jour les variables selon la categorie et l option
    if (category == SETTINGS_CATEGORY_SOUND) {
        if (selectedItem == 0) ringVolume = options[0].value;
        if (selectedItem == 1) ringTone = options[1].value;
        if (selectedItem == 2) vibrateEnabled = options[2].value;
        if (selectedItem == 3) silentMode = options[3].value;
    }
    if (category == SETTINGS_CATEGORY_DISPLAY) {
        if (selectedItem == 0) brightness = options[0].value;
        if (selectedItem == 1) screenTimeout = options[1].value;
    }
    if (category == SETTINGS_CATEGORY_NETWORK) {
        if (selectedItem == 0) frequencyIndex = options[0].value;
        if (selectedItem == 1) txPower = options[1].value;
    }
    if (category == SETTINGS_CATEGORY_SECURITY) {
        if (selectedItem == 1) encryptionEnabled = options[1].value;
    }
    if (category == SETTINGS_CATEGORY_POWER) {
        if (selectedItem == 0) powerSavingEnabled = options[0].value;
        if (selectedItem == 1) sleepTimeout = options[1].value;
    }
}

// ============================================
// ACCES AUX REGLAGES
// ============================================
uint8_t SettingsScreen::getVolume(void) { return ringVolume; }
uint8_t SettingsScreen::getBrightness(void) { return brightness; }
bool SettingsScreen::isSilentMode(void) { return silentMode; }
bool SettingsScreen::isVibrateEnabled(void) { return vibrateEnabled; }
uint8_t SettingsScreen::getFrequencyIndex(void) { return frequencyIndex; }
uint8_t SettingsScreen::getTxPower(void) { return txPower; }
bool SettingsScreen::isEncryptionEnabled(void) { return encryptionEnabled; }
bool SettingsScreen::isActiveScreen(void) { return isActive; }

// ============================================
// FIN DU FICHIER settings_screen.cpp
// ============================================