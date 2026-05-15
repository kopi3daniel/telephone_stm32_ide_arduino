 /**
 
 * Ce fichier contient TOUT le code graphique du téléphone.
 * Chaque fonction est commentée avec ce qu'elle fait et pourquoi.
 * 
 * Bibliothèques requises :
 * - Adafruit GFX Library
 * - Adafruit SSD1306
 */

#include "display.h"
#include <string.h>
#include <stdio.h>

// ============================================
// BITMAPS DES ICÔNES (8x8 pixels, 1 bit/pixel)
// ============================================
// Chaque octet représente 8 pixels verticaux d'une colonne
// Le bit 0 = pixel du bas, bit 7 = pixel du haut
// Colonne 0 à gauche, colonne 7 à droite

// --- SIGNAL LORA (5 niveaux) ---

// 0 barre : juste l'antenne
const uint8_t Display::signal_0[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E
};

// 1 barre : antenne + 1 barre
const uint8_t Display::signal_1[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x7E
};

// 2 barres
const uint8_t Display::signal_2[8] = {
    0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x7E
};

// 3 barres
const uint8_t Display::signal_3[8] = {
    0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E
};

// 4 barres (signal excellent)
const uint8_t Display::signal_4[8] = {
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E
};

// --- BATTERIE (6 états) ---

// Batterie pleine (100%)
const uint8_t Display::battery_full[8] = {
    0x7E, 0x42, 0x5A, 0x5A, 0x5A, 0x5A, 0x42, 0x7E
};

// Batterie 75%
const uint8_t Display::battery_75[8] = {
    0x7E, 0x42, 0x5A, 0x5A, 0x5A, 0x42, 0x42, 0x7E
};

// Batterie 50%
const uint8_t Display::battery_50[8] = {
    0x7E, 0x42, 0x5A, 0x5A, 0x42, 0x42, 0x42, 0x7E
};

// Batterie 25%
const uint8_t Display::battery_25[8] = {
    0x7E, 0x42, 0x5A, 0x42, 0x42, 0x42, 0x42, 0x7E
};

// Batterie vide (0%)
const uint8_t Display::battery_empty[8] = {
    0x7E, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7E
};

// Batterie en charge (éclair au milieu)
const uint8_t Display::battery_charge[8] = {
    0x00, 0x08, 0x18, 0x38, 0x7E, 0x3C, 0x18, 0x10
};

// --- ICÔNES DE NOTIFICATION ---

// Nouveau message (enveloppe)
const uint8_t Display::icon_message[8] = {
    0x00, 0x7E, 0x42, 0x5A, 0x42, 0x24, 0x18, 0x00
};

// Appel manqué (téléphone avec flèche)
const uint8_t Display::icon_call_missed[8] = {
    0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00
};

// Torche allumée
const uint8_t Display::icon_torch[8] = {
    0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x00
};

// Cadenas (verrouillage)
const uint8_t Display::icon_lock[8] = {
    0x00, 0x3C, 0x42, 0x42, 0x7E, 0x7E, 0x7E, 0x00
};

// Mode silencieux (haut-parleur barré)
const uint8_t Display::icon_silent[8] = {
    0x00, 0x10, 0x18, 0x3C, 0x3C, 0x18, 0x10, 0x00
};

// Micro coupé
const uint8_t Display::icon_mute[8] = {
    0x00, 0x00, 0x24, 0x18, 0x24, 0x00, 0x00, 0x00
};

// Haut-parleur actif
const uint8_t Display::icon_speaker[8] = {
    0x00, 0x10, 0x18, 0x7E, 0x7E, 0x18, 0x10, 0x00
};

// ============================================
// CONSTRUCTEUR
// ============================================
// Initialise l'objet écran avec les bonnes dimensions
// L'initialisation Wire se fait dans init()
Display::Display() 
    : oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1)  // -1 = pas de pin reset dédiée
    , currentScreen(SCREEN_HOME)  // Démarre sur l'écran d'accueil
    , screenOn(true)       // Écran allumé au démarrage
    , brightness(255)      // Luminosité maximale
    , currentRssi(-85)
    , currentBatteryPercent(85)
    , currentBatteryCharging(false)
    , currentHour(12)
    , currentMinute(34)
    , missedCallIcon(false)
    , newMessageIcon(false)
    , silentIcon(false)
    , torchIcon(false) {
}

// ============================================
// INITIALISATION DU MATÉRIEL
// ============================================
// Configure le bus I2C et initialise l'écran OLED
// Doit être appelée UNE FOIS au démarrage
void Display::init(void) {
    
    // Initialiser le bus I2C avec les broches définies dans config.h
    Wire.setSCL(OLED_SCL_PIN);
    Wire.setSDA(OLED_SDA_PIN);
    Wire.begin();
    
    // Tenter d'initialiser l'écran OLED
    // SSD1306_SWITCHCAPVCC = génère la tension d'alimentation interne
    if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        // === ÉCHEC D'INITIALISATION ===
        // L'écran n'a pas répondu (problème de câblage ?)
        // On signale l'erreur en faisant clignoter la LED torche
        pinMode(TORCH_PIN, OUTPUT);
        while (1) {
            digitalToggle(TORCH_PIN);  // Inverse la LED
            delay(200);                // Attend 200ms
        }
        // Le programme est BLOQUÉ ici tant que l'écran ne fonctionne pas
    }
    
    // === SUCCÈS : l'écran est détecté ===
    
    // Configuration de base de l'affichage
    oled.clearDisplay();          // Efface tout
    oled.setTextSize(1);          // Taille de texte : petite (6x8 pixels)
    oled.setTextColor(SSD1306_WHITE);  // Texte en blanc
    oled.setTextWrap(false);      // Pas de retour à la ligne automatique
    oled.display();               // Envoie à l'écran
    
    // Afficher le logo de démarrage
    showSplashScreen();
}

// ============================================
// ÉCRAN DE DÉMARRAGE (SPLASH SCREEN)
// ============================================
// Affiche "PHONE LORA" avec la version pendant 1.5 secondes
void Display::showSplashScreen(void) {
    oled.clearDisplay();
    
    // --- Titre "PHONE" en grand ---
    oled.setTextSize(2);  // Grande police (12x16 pixels par caractère)
    
    // Calculer la largeur du texte pour le centrer
    int16_t x1, y1;
    uint16_t w, h;
    oled.getTextBounds("PHONE", 0, 0, &x1, &y1, &w, &h);
    oled.setCursor((W - w) / 2, 5);  // Centré horizontalement, 5px du haut
    oled.print("PHONE");
    
    // --- Sous-titre "LORA" en grand ---
    oled.getTextBounds("LORA", 0, 0, &x1, &y1, &w, &h);
    oled.setCursor((W - w) / 2, 25);  // 25px du haut
    oled.print("LORA");
    
    // --- Version en petit ---
    oled.setTextSize(1);  // Petite police
    oled.setCursor((W - 50) / 2, 50);  // En bas
    oled.print("v1.0.0");
    
    oled.display();
    delay(1500);  // Affiche 1.5 secondes
    
    // Effacer pour passer à l'écran d'accueil
    clear();
}

// ============================================
// CONTRÔLE DE L'ALIMENTATION
// ============================================

void Display::powerOn(void) {
    screenOn = true;
    // Envoyer la commande DISPLAYON au contrôleur SSD1306
    oled.ssd1306_command(SSD1306_DISPLAYON);
}

void Display::powerOff(void) {
    screenOn = false;
    // Envoyer la commande DISPLAYOFF (écran en veille, économie 90% énergie)
    oled.ssd1306_command(SSD1306_DISPLAYOFF);
}

void Display::setBrightness(uint8_t level) {
    brightness = level;
    // La luminosité est contrôlée par le contraste sur SSD1306
    oled.ssd1306_command(SSD1306_SETCONTRAST);
    oled.ssd1306_command(level);
}

void Display::clear(void) {
    oled.clearDisplay();  // Efface le buffer local
}

void Display::refresh(void) {
    oled.display();       // Envoie le buffer à l'écran
    oled.clearDisplay();  // Prépare le buffer pour le prochain affichage
}

// ============================================
// DESSIN D'UNE ICÔNE BITMAP
// ============================================
// Fonction interne qui appelle la méthode drawBitmap d'Adafruit
void Display::drawIcon(uint8_t x, uint8_t y, const uint8_t* bitmap, 
                      uint8_t w, uint8_t h) {
    oled.drawBitmap(x, y, bitmap, w, h, SSD1306_WHITE);
}

// ============================================
// TEXTE CENTRÉ HORIZONTALEMENT
// ============================================
// Calcule la largeur du texte et le positionne au centre
void Display::drawCenteredText(uint8_t y, const char* text, uint8_t size) {
    int16_t x1, y1;
    uint16_t w, h;
    oled.setTextSize(size);
    oled.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    oled.setCursor((W - w) / 2, y);
    oled.print(text);
}

// ============================================
// SÉLECTION DE L'ICÔNE DE SIGNAL
// ============================================
// Retourne le bon bitmap selon la force du signal
const uint8_t* Display::getSignalIcon(int8_t rssi) {
    // Le RSSI LoRa varie de -120 dBm (très faible) à 0 dBm (très fort)
    if (rssi > -70)  return signal_4;   // Excellent (> -70 dBm)
    if (rssi > -85)  return signal_3;   // Bon     (> -85 dBm)
    if (rssi > -100) return signal_2;   // Moyen   (> -100 dBm)
    if (rssi > -115) return signal_1;   // Faible  (> -115 dBm)
    return signal_0;                     // Très faible
}

// ============================================
// SÉLECTION DE L'ICÔNE DE BATTERIE
// ============================================
const uint8_t* Display::getBatteryIcon(uint8_t percent, bool charging) {
    if (charging) return battery_charge;  // En charge : icône éclair
    if (percent > 75) return battery_full;
    if (percent > 50) return battery_75;
    if (percent > 25) return battery_50;
    if (percent > 5)  return battery_25;
    return battery_empty;  // Presque vide
}

// ============================================
// DESSIN DE LA BARRE D'ÉTAT (8 pixels en haut)
// ============================================
void Display::drawStatusBar(void) {
    // Ligne de séparation sous la barre d'état
    oled.drawLine(0, 8, W, 8, SSD1306_WHITE);
    
    // Redessiner toutes les informations
    updateSignal(currentRssi);
    updateBattery(currentBatteryPercent, currentBatteryCharging);
    updateTime(currentHour, currentMinute);
    
    // Redessiner les icônes de notification si actives
    if (missedCallIcon) drawIcon(18, 0, icon_call_missed, 8, 8);
    if (newMessageIcon) drawIcon(28, 0, icon_message, 8, 8);
    if (silentIcon)     drawIcon(38, 0, icon_silent, 8, 8);
    if (torchIcon)      drawIcon(48, 0, icon_torch, 8, 8);
}

// ============================================
// MISE À JOUR DU SIGNAL DANS LA BARRE
// ============================================
void Display::updateSignal(int8_t rssi) {
    currentRssi = rssi;
    
    // Effacer la zone du signal (coin supérieur gauche)
    oled.fillRect(0, 0, 24, 8, SSD1306_BLACK);
    
    // Dessiner l'icône correspondante
    const uint8_t* icon = getSignalIcon(rssi);
    drawIcon(0, 0, icon, 8, 8);
    
    // Afficher la valeur en dBm à côté
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(10, 0);
    
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", rssi);
    oled.print(buf);
}

// ============================================
// MISE À JOUR DE LA BATTERIE DANS LA BARRE
// ============================================
void Display::updateBattery(uint8_t percent, bool charging) {
    currentBatteryPercent = percent;
    currentBatteryCharging = charging;
    
    // Position : vers le milieu-droit
    uint8_t batX = 70;
    
    // Effacer la zone
    oled.fillRect(batX, 0, 30, 8, SSD1306_BLACK);
    
    // Dessiner l'icône
    const uint8_t* icon = getBatteryIcon(percent, charging);
    drawIcon(batX, 0, icon, 8, 8);
    
    // Afficher le pourcentage
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(batX + 10, 0);
    
    char buf[5];
    snprintf(buf, sizeof(buf), "%d%%", percent);
    oled.print(buf);
}

// ============================================
// MISE À JOUR DE L'HEURE DANS LA BARRE
// ============================================
void Display::updateTime(uint8_t hour, uint8_t minute) {
    currentHour = hour;
    currentMinute = minute;
    
    // Position : tout à droite
    uint8_t timeX = 100;
    
    // Effacer la zone
    oled.fillRect(timeX, 0, 28, 8, SSD1306_BLACK);
    
    // Afficher l'heure au format HH:MM
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(timeX, 0);
    
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
    oled.print(buf);
}

// ============================================
// ICÔNES DE NOTIFICATION
// ============================================
void Display::showMissedCallIcon(bool show) {
    missedCallIcon = show;
    if (show) {
        drawIcon(18, 0, icon_call_missed, 8, 8);
    } else {
        oled.fillRect(18, 0, 8, 8, SSD1306_BLACK);
    }
}

void Display::showNewMessageIcon(bool show) {
    newMessageIcon = show;
    if (show) {
        drawIcon(28, 0, icon_message, 8, 8);
    } else {
        oled.fillRect(28, 0, 8, 8, SSD1306_BLACK);
    }
}

void Display::showSilentIcon(bool show) {
    silentIcon = show;
    if (show) {
        drawIcon(38, 0, icon_silent, 8, 8);
    } else {
        oled.fillRect(38, 0, 8, 8, SSD1306_BLACK);
    }
}

void Display::showTorchIcon(bool show) {
    torchIcon = show;
    if (show) {
        drawIcon(48, 0, icon_torch, 8, 8);
    } else {
        oled.fillRect(48, 0, 8, 8, SSD1306_BLACK);
    }
}

// ============================================
// ÉCRAN D'ACCUEIL
// ============================================
void Display::showHomeScreen(void) {
    currentScreen = SCREEN_HOME;
    oled.clearDisplay();
    
    // --- Barre d'état ---
    drawStatusBar();
    
    // --- Horloge centrale ---
    oled.setTextSize(3);  // Très grande police
    
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", currentHour, currentMinute);
    drawCenteredText(20, timeStr, 3);
    
    // --- Date ---
    oled.setTextSize(1);
    drawCenteredText(44, "Lun 15 Jan", 1);
    
    // --- Barre du bas : touches programmables ---
    oled.setTextSize(1);
    oled.setCursor(0, 56);
    oled.print("Menu");
    oled.setCursor(55, 56);
    oled.print("Contacts");
    oled.setCursor(110, 56);
    oled.print("Appels");
    
    oled.display();
}

// ============================================
// ÉCRAN COMPOSEUR (NUMÉROTATION)
// ============================================
void Display::showDialerScreen(const char* number) {
    currentScreen = SCREEN_DIALER;
    oled.clearDisplay();
    
    // --- Barre d'état ---
    drawStatusBar();
    
    // --- Numéro composé en grand ---
    oled.setTextSize(2);
    oled.setCursor(5, 15);
    oled.print(number);
    
    // --- Ligne de séparation ---
    oled.drawLine(0, 38, W, 38, SSD1306_WHITE);
    
    // --- Suggestions (si disponible) ---
    oled.setTextSize(1);
    oled.setCursor(5, 42);
    oled.print("Saisir numero...");
    
    // --- Barre du bas ---
    oled.setCursor(0, 56);
    oled.print("Effacer");
    oled.setCursor(110, 56);
    oled.print("Appeler");
    
    oled.display();
}

void Display::showDialerSuggestions(const char* suggestions[], uint8_t count) {
    // Afficher sous le numéro
    for (uint8_t i = 0; i < count && i < 3; i++) {
        oled.setCursor(5, 42 + (i * 8));
        oled.print(suggestions[i]);
    }
    oled.display();
}

// ============================================
// ÉCRAN APPEL SORTANT
// ============================================
void Display::showOutgoingCall(const char* name, const char* number) {
    currentScreen = SCREEN_CALL_ACTIVE;
    oled.clearDisplay();
    
    drawStatusBar();
    
    // --- Titre ---
    drawCenteredText(15, "Appel en cours...", 1);
    
    // --- Nom du contact ---
    oled.setTextSize(2);
    drawCenteredText(28, name, 2);
    
    // --- Numéro ---
    oled.setTextSize(1);
    drawCenteredText(42, number, 1);
    
    // --- Statut ---
    oled.setTextSize(1);
    oled.setCursor(10, 50);
    oled.print("Sonnerie...");
    
    // --- Bouton raccrocher ---
    oled.setCursor(110, 56);
    oled.print("Raccrocher");
    
    oled.display();
}

// ============================================
// ÉCRAN APPEL ENTRANT
// ============================================
void Display::showIncomingCall(const char* name, const char* number) {
    currentScreen = SCREEN_INCOMING_CALL;
    oled.clearDisplay();
    
    drawStatusBar();
    
    // --- Titre ---
    drawCenteredText(10, "APPEL ENTRANT", 1);
    
    // --- Nom en grand ---
    oled.setTextSize(2);
    drawCenteredText(25, name, 2);
    
    // --- Numéro ---
    oled.setTextSize(1);
    drawCenteredText(40, number, 1);
    
    // --- Instructions ---
    oled.setCursor(5, 52);
    oled.print("VERT: Decrocher");
    oled.setCursor(80, 52);
    oled.print("ROUGE: Refuser");
    
    oled.display();
}

// ============================================
// ÉCRAN APPEL ACTIF
// ============================================
void Display::showCallActive(const char* name, uint32_t duration_sec, bool muted) {
    currentScreen = SCREEN_CALL_ACTIVE;
    oled.clearDisplay();
    
    drawStatusBar();
    
    // --- Nom ---
    oled.setTextSize(1);
    drawCenteredText(12, "En communication", 1);
    
    oled.setTextSize(2);
    drawCenteredText(22, name, 2);
    
    // --- Durée ---
    oled.setTextSize(1);
    uint32_t min = duration_sec / 60;
    uint32_t sec = duration_sec % 60;
    
    char dur[10];
    snprintf(dur, sizeof(dur), "%02lu:%02lu", min, sec);
    drawCenteredText(38, dur, 1);
    
    // --- Icône muet si actif ---
    if (muted) {
        drawIcon(60, 45, icon_mute, 8, 8);
        oled.setCursor(70, 45);
        oled.print("Muet");
    }
    
    // --- Instructions ---
    oled.setCursor(5, 55);
    oled.print("ROUGE: Raccrocher");
    
    oled.display();
}

// ============================================
// MISE À JOUR DE LA DURÉE (appel actif)
// ============================================
void Display::updateCallDuration(uint32_t duration_sec) {
    // Ne redessiner que la durée (zone spécifique)
    oled.fillRect(50, 36, 30, 10, SSD1306_BLACK);
    
    uint32_t min = duration_sec / 60;
    uint32_t sec = duration_sec % 60;
    
    char dur[10];
    snprintf(dur, sizeof(dur), "%02lu:%02lu", min, sec);
    drawCenteredText(38, dur, 1);
    
    oled.display();
}

// ============================================
// ÉCRAN APPEL TERMINÉ
// ============================================
void Display::showCallEnded(const char* name, uint32_t duration_sec) {
    currentScreen = SCREEN_CALL_ENDED;
    oled.clearDisplay();
    
    // --- Titre ---
    drawCenteredText(10, "Appel termine", 1);
    
    // --- Nom ---
    oled.setTextSize(1);
    drawCenteredText(25, name, 1);
    
    // --- Durée finale ---
    uint32_t min = duration_sec / 60;
    uint32_t sec = duration_sec % 60;
    
    char dur[20];
    snprintf(dur, sizeof(dur), "Duree: %02lu:%02lu", min, sec);
    drawCenteredText(40, dur, 1);
    
    oled.display();
    delay(2000);  // Afficher 2 secondes avant de revenir
}

// ============================================
// ÉCRAN ÉDITEUR DE MESSAGE
// ============================================
void Display::showMessageEditor(const char* text, const char* mode) {
    currentScreen = SCREEN_MESSAGE_EDITOR;
    oled.clearDisplay();
    
    drawStatusBar();
    
    // --- Titre ---
    drawCenteredText(10, "Nouveau message", 1);
    
    // --- Ligne séparation ---
    oled.drawLine(0, 18, W, 18, SSD1306_WHITE);
    
    // --- Texte saisi ---
    oled.setTextSize(1);
    oled.setCursor(2, 22);
    oled.print(text);
    
    // --- Curseur clignotant ---
    // (géré par la boucle principale avec un timer)
    
    // --- Mode de saisie ---
    oled.setCursor(2, 50);
    oled.print("Mode: ");
    oled.print(mode);
    
    // --- Instructions ---
    oled.setCursor(2, 56);
    oled.print("VERT:Envoyer  ROUGE:Annuler");
    
    oled.display();
}

void Display::showMessagePreview(char currentChar) {
    // Afficher temporairement le caractère en cours
    oled.fillRect(110, 45, 15, 10, SSD1306_BLACK);
    oled.setCursor(110, 45);
    oled.print(currentChar);
    oled.display();
}

void Display::showMessageSent(bool success) {
    oled.clearDisplay();
    
    if (success) {
        drawCenteredText(25, "Message envoye !", 2);
    } else {
        drawCenteredText(25, "Echec envoi", 2);
    }
    
    oled.display();
    delay(1500);
}

// ============================================
// LISTE DES CONVERSATIONS
// ============================================
void Display::showMessageList(const char* contacts[], const char* previews[],
                             uint8_t count, uint8_t selected) {
    currentScreen = SCREEN_MESSAGE_LIST;
    oled.clearDisplay();
    
    drawStatusBar();
    drawCenteredText(10, "Messages", 1);
    oled.drawLine(0, 18, W, 18, SSD1306_WHITE);
    
    // Afficher les conversations (max 5 visibles)
    for (uint8_t i = 0; i < count && i < 5; i++) {
        uint8_t y = 20 + (i * 8);
        
        if (i == selected) {
            // Surligner la sélection
            oled.fillRect(0, y, W, 8, SSD1306_WHITE);
            oled.setTextColor(SSD1306_BLACK);
        } else {
            oled.setTextColor(SSD1306_WHITE);
        }
        
        oled.setCursor(2, y);
        oled.print(contacts[i]);
        
        // Aperçu du dernier message
        if (previews[i] != NULL) {
            oled.setCursor(60, y);
            oled.print(previews[i]);
        }
    }
    
    oled.setTextColor(SSD1306_WHITE);
    oled.display();
}

// ============================================
// LISTE DES CONTACTS
// ============================================
void Display::showContactsList(const char* names[], uint8_t count, uint8_t selected) {
    currentScreen = SCREEN_CONTACTS_LIST;
    oled.clearDisplay();
    
    drawStatusBar();
    drawCenteredText(10, "Contacts", 1);
    oled.drawLine(0, 18, W, 18, SSD1306_WHITE);
    
    for (uint8_t i = 0; i < count && i < 5; i++) {
        uint8_t y = 20 + (i * 9);
        
        if (i == selected) {
            oled.fillRect(0, y, W, 9, SSD1306_WHITE);
            oled.setTextColor(SSD1306_BLACK);
        } else {
            oled.setTextColor(SSD1306_WHITE);
        }
        
        oled.setCursor(2, y);
        oled.print(names[i]);
    }
    
    oled.setTextColor(SSD1306_WHITE);
    
    // Barre du bas
    oled.setCursor(0, 56);
    oled.print("Appeler");
    oled.setCursor(80, 56);
    oled.print("Options");
    
    oled.display();
}

// ============================================
// DÉTAIL D'UN CONTACT
// ============================================
void Display::showContactDetails(const char* name, const char* number, bool favorite) {
    currentScreen = SCREEN_CONTACT_VIEW;
    oled.clearDisplay();
    
    drawStatusBar();
    
    // --- Nom ---
    oled.setTextSize(2);
    drawCenteredText(15, name, 2);
    
    // --- Favori ---
    if (favorite) {
        oled.setTextSize(1);
        oled.setCursor(100, 15);
        oled.print("*");
    }
    
    // --- Numéro ---
    oled.setTextSize(1);
    drawCenteredText(35, number, 1);
    
    // --- Options ---
    oled.setCursor(5, 50);
    oled.print("VERT:Appeler");
    oled.setCursor(85, 50);
    oled.print("C:Modifier");
    
    oled.display();
}

// ============================================
// MENU RÉGLAGES
// ============================================
void Display::showSettingsMenu(const char* items[], uint8_t count, uint8_t selected) {
    currentScreen = SCREEN_SETTINGS_MENU;
    oled.clearDisplay();
    
    drawStatusBar();
    drawCenteredText(10, "Reglages", 1);
    oled.drawLine(0, 18, W, 18, SSD1306_WHITE);
    
    for (uint8_t i = 0; i < count && i < 5; i++) {
        uint8_t y = 20 + (i * 9);
        
        if (i == selected) {
            oled.fillRect(0, y, W, 9, SSD1306_WHITE);
            oled.setTextColor(SSD1306_BLACK);
        } else {
            oled.setTextColor(SSD1306_WHITE);
        }
        
        oled.setCursor(5, y);
        oled.print(items[i]);
    }
    
    oled.setTextColor(SSD1306_WHITE);
    oled.display();
}

// ============================================
// SAISIE CODE PIN
// ============================================
void Display::showPinInput(uint8_t digits) {
    currentScreen = SCREEN_LOCK;
    oled.clearDisplay();
    
    drawCenteredText(10, "Code PIN", 1);
    drawCenteredText(30, "____", 2);
    
    // Afficher les digits saisis
    oled.setTextSize(2);
    oled.setCursor(40, 30);
    for (uint8_t i = 0; i < digits; i++) {
        oled.print("*");
    }
    
    oled.setCursor(15, 50);
    oled.print("C:Effacer");
    
    oled.display();
}

void Display::showPinError(void) {
    oled.clearDisplay();
    drawCenteredText(20, "Code incorrect !", 1);
    drawCenteredText(35, "Reessayez...", 1);
    oled.display();
    delay(1500);
}

// ============================================
// BOÎTES DE DIALOGUE
// ============================================
void Display::showAlert(const char* title, const char* message) {
    // Fond grisé (quadrillage)
    for (uint8_t y = 10; y < 54; y += 2) {
        for (uint8_t x = 10; x < 118; x += 2) {
            oled.drawPixel(x, y, SSD1306_WHITE);
        }
    }
    
    // Rectangle du dialogue
    oled.fillRect(10, 10, 108, 44, SSD1306_BLACK);
    oled.drawRect(10, 10, 108, 44, SSD1306_WHITE);
    
    oled.setTextSize(1);
    drawCenteredText(14, title, 1);
    drawCenteredText(30, message, 1);
    
    oled.setCursor(50, 48);
    oled.print("OK");
    
    oled.display();
}

void Display::showLowBattery(uint8_t percent) {
    oled.clearDisplay();
    
    drawIcon(60, 10, battery_empty, 8, 8);
    
    char msg[30];
    snprintf(msg, sizeof(msg), "Batterie faible: %d%%", percent);
    drawCenteredText(30, msg, 1);
    drawCenteredText(45, "Rechargez SVP", 1);
    
    oled.display();
    delay(3000);
}

// ============================================
// FONCTIONS DE DESSIN BAS NIVEAU
// ============================================
void Display::drawText(uint8_t x, uint8_t y, const char* text, uint8_t size) {
    oled.setTextSize(size);
    oled.setCursor(x, y);
    oled.print(text);
}

void Display::drawInvertedText(uint8_t x, uint8_t y, const char* text, uint8_t size) {
    // Simuler texte inversé : fond blanc, texte noir
    int16_t x1, y1;
    uint16_t w, h;
    oled.setTextSize(size);
    oled.getTextBounds(text, x, y, &x1, &y1, &w, &h);
    
    oled.fillRect(x, y, w, h, SSD1306_WHITE);
    oled.setTextColor(SSD1306_BLACK);
    oled.setCursor(x, y);
    oled.print(text);
    oled.setTextColor(SSD1306_WHITE);
}

void Display::drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    oled.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
}

void Display::drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool filled) {
    if (filled) {
        oled.fillRect(x, y, w, h, SSD1306_WHITE);
    } else {
        oled.drawRect(x, y, w, h, SSD1306_WHITE);
    }
}

void Display::drawProgressBar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, 
                             uint8_t percent) {
    oled.drawRect(x, y, w, h, SSD1306_WHITE);
    uint8_t fillW = (w - 2) * percent / 100;
    oled.fillRect(x + 1, y + 1, fillW, h - 2, SSD1306_WHITE);
}

// ============================================
// GETTERS / SETTERS
// ============================================
ScreenType_t Display::getCurrentScreen(void) {
    return currentScreen;
}

void Display::setScreen(ScreenType_t screen) {
    currentScreen = screen;
}

bool Display::isScreenOn(void) {
    return screenOn;
}

// ============================================
// FIN DU FICHIER display.cpp
// ============================================
// Résumé de l'interface écran :
// ✅ 13 écrans différents
// ✅ Barre d'état avec icônes
// ✅ Icônes bitmap (signal, batterie, notifications)
// ✅ Boîtes de dialogue
// ✅ Gestion luminosité et veille
// ✅ Texte centré, inversé
// ✅ Barres de progression
//
// TOTAL : ~600 lignes de code commenté