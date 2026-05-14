/**
 * ---------------------------------------------------------------------------
 * menu.h - Widget de menu pour le Telephone LoRa
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Ce widget gere l affichage et la navigation dans les menus.
 * Il est utilise par tous les ecrans qui ont besoin d une liste
 * d options a faire defiler et a selectionner.
 * 
 * Fonctionnalites :
 * - Liste d options avec defilement vertical
 * - Surbrillance de l option selectionnee
 * - Icones optionnelles a gauche
 * - Valeurs/indicateurs a droite
 * - Barre de defilement
 * - Gestion des sous-menus
 */

#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include "config.h"

// ============================================
// TYPES D ELEMENTS DE MENU
// ============================================
typedef enum {
    MENU_ITEM_NORMAL,           // Element normal (texte simple)
    MENU_ITEM_TOGGLE,           // Element avec interrupteur ON/OFF
    MENU_ITEM_SLIDER,           // Element avec barre de valeur
    MENU_ITEM_SUBMENU,          // Element menant a un sous-menu
    MENU_ITEM_ACTION,           // Element qui declenche une action
    MENU_ITEM_SEPARATOR         // Ligne de separation
} MenuItemType_t;

// ============================================
// STRUCTURE D UN ELEMENT DE MENU
// ============================================
typedef struct {
    const char* label;          // Texte principal
    const char* sublabel;       // Texte secondaire (optionnel)
    const char* value;          // Valeur affichee a droite
    MenuItemType_t type;        // Type d element
    uint8_t iconIndex;          // Index de l icone (0 = pas d icone)
    bool enabled;               // Element active ?
    bool toggleState;           // Etat ON/OFF (pour TOGGLE)
    uint8_t sliderValue;        // Valeur du curseur (pour SLIDER)
    uint8_t sliderMax;          // Valeur max du curseur
    void (*action)(void);       // Fonction a executer (pour ACTION)
} MenuItem_t;

// ============================================
// CONFIGURATION DU MENU
// ============================================
#define MENU_MAX_ITEMS          20      // Nombre max d elements
#define MENU_VISIBLE_ITEMS      5       // Elements visibles a l ecran
#define MENU_ITEM_HEIGHT        10      // Hauteur d un element (pixels)
#define MENU_SCROLLBAR_WIDTH    2       // Largeur barre de defilement

// ============================================
// CLASSE MENU
// ============================================
class Menu {
    
private:
    // ============================================
    // ELEMENTS DU MENU
    // ============================================
    MenuItem_t items[MENU_MAX_ITEMS];   // Tableau des elements
    uint8_t itemCount;                   // Nombre d elements
    uint8_t selectedIndex;               // Index selectionne
    uint8_t scrollOffset;                // Decalage de defilement
    uint8_t visibleCount;                // Nombre visible
    
    // ============================================
    // CONFIGURATION VISUELLE
    // ============================================
    uint8_t menuX;                       // Position X du menu
    uint8_t menuY;                       // Position Y du debut
    uint8_t menuWidth;                   // Largeur du menu
    uint8_t menuHeight;                  // Hauteur totale visible
    
    char title[32];                      // Titre du menu
    bool showTitle;                      // Afficher le titre ?
    bool showScrollbar;                  // Afficher la barre de defilement ?
    
    // ============================================
    // ETAT
    // ============================================
    bool isActive;                       // Menu actif ?
    bool needsRedraw;                    // Doit etre redessine ?
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Dessine un element du menu
    void drawItem(uint8_t index, uint8_t y, bool selected);
    
    // Dessine la barre de titre
    void drawTitle(void);
    
    // Dessine la barre de defilement
    void drawScrollbar(void);
    
    // Dessine une icone pour un element
    void drawItemIcon(uint8_t index, uint8_t x, uint8_t y);
    
    // Dessine la valeur d un element (droite)
    void drawItemValue(uint8_t index, uint8_t x, uint8_t y);
    
    // Dessine un interrupteur ON/OFF
    void drawToggle(uint8_t x, uint8_t y, bool state);
    
    // Dessine un curseur de valeur
    void drawSlider(uint8_t x, uint8_t y, uint8_t value, uint8_t max);
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    Menu();
    void init(void);
    
    // ============================================
    // CONFIGURATION
    // ============================================
    
    // Definit la position et taille du menu
    void setBounds(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
    
    // Definit le titre
    void setTitle(const char* menuTitle);
    
    // Affiche/masque le titre
    void showTitleBar(bool show);
    
    // Affiche/masque la barre de defilement
    void showScrollBar(bool show);
    
    // ============================================
    // GESTION DES ELEMENTS
    // ============================================
    
    // Ajoute un element simple
    void addItem(const char* label, MenuItemType_t type);
    
    // Ajoute un element avec sous-texte
    void addItemWithSub(const char* label, const char* sublabel);
    
    // Ajoute un element avec valeur
    void addItemWithValue(const char* label, const char* value);
    
    // Ajoute un element toggle
    void addToggle(const char* label, bool initialState);
    
    // Ajoute un element slider
    void addSlider(const char* label, uint8_t value, uint8_t max);
    
    // Ajoute un element action
    void addAction(const char* label, void (*action)(void));
    
    // Ajoute un separateur
    void addSeparator(void);
    
    // Supprime tous les elements
    void clearAll(void);
    
    // Retourne le nombre d elements
    uint8_t getItemCount(void);
    
    // ============================================
    // NAVIGATION
    // ============================================
    
    // Deplace la selection vers le haut
    void moveUp(void);
    
    // Deplace la selection vers le bas
    void moveDown(void);
    
    // Selectionne un element par index
    void selectItem(uint8_t index);
    
    // Active l element selectionne (execute l action si ACTION)
    void activateSelected(void);
    
    // Retourne l index selectionne
    uint8_t getSelectedIndex(void);
    
    // Retourne le type de l element selectionne
    MenuItemType_t getSelectedType(void);
    
    // ============================================
    // MANIPULATION DES ELEMENTS
    // ============================================
    
    // Active/desactive un element
    void setItemEnabled(uint8_t index, bool enabled);
    
    // Modifie le label d un element
    void setItemLabel(uint8_t index, const char* label);
    
    // Modifie la valeur d un element
    void setItemValue(uint8_t index, const char* value);
    
    // Bascule un element toggle
    void toggleItem(uint8_t index);
    
    // Modifie la valeur d un slider
    void setSliderValue(uint8_t index, uint8_t value);
    
    // Retourne l etat d un toggle
    bool getToggleState(uint8_t index);
    
    // Retourne la valeur d un slider
    uint8_t getSliderValue(uint8_t index);
    
    // ============================================
    // AFFICHAGE
    // ============================================
    
    // Dessine le menu complet
    void draw(void);
    
    // Active/desactive le menu
    void setActive(bool active);
    
    // Force le redessin
    void forceRedraw(void);
    
    // Verifie si le menu est actif
    bool isActiveMenu(void);
};

#endif // MENU_H