/**
 * ---------------------------------------------------------------------------
 * contacts_screen.h - Ecran du repertoire de contacts du Telephone LoRa
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Cet ecran gere l affichage et la navigation dans le repertoire.
 * Fonctionnalites :
 * - Liste des contacts avec defilement
 * - Recherche rapide par nom
 * - Affichage detaille d un contact
 * - Ajout/Modification/Suppression de contact
 * - Appel direct depuis un contact
 * - Envoi de SMS depuis un contact
 * - Gestion des favoris
 */

#ifndef CONTACTS_SCREEN_H
#define CONTACTS_SCREEN_H

#include <Arduino.h>
#include "config.h"

// ============================================
// MODES DE L ECRAN CONTACTS
// ============================================
typedef enum {
    CONTACTS_MODE_LIST,         // Liste des contacts
    CONTACTS_MODE_VIEW,         // Vue detaillee d un contact
    CONTACTS_MODE_EDIT,         // Edition d un contact
    CONTACTS_MODE_ADD,          // Ajout d un nouveau contact
    CONTACTS_MODE_DELETE,       // Confirmation suppression
    CONTACTS_MODE_SEARCH        // Recherche par nom
} ContactsMode_t;

// ============================================
// CHAMP EN EDITION
// ============================================
typedef enum {
    EDIT_FIELD_NAME,            // Edition du nom
    EDIT_FIELD_NUMBER,          // Edition du numero
    EDIT_FIELD_COUNT
} EditField_t;

// ============================================
// STRUCTURE D UN CONTACT
// ============================================
typedef struct {
    char name[32];              // Nom du contact
    char number[16];            // Numero de telephone
    bool isFavorite;            // Contact favori ?
    uint8_t index;              // Index dans la liste principale
} ContactInfo_t;

// ============================================
// NOMBRE MAX DE CONTACTS AFFICHABLES
// ============================================
#define MAX_CONTACTS_DISPLAY    100
#define CONTACTS_PER_PAGE       6       // Nombre visible sur l ecran

// ============================================
// CLASSE CONTACTS SCREEN
// ============================================
class ContactsScreen {
    
private:
    // ============================================
    // ETAT DE L ECRAN
    // ============================================
    ContactsMode_t mode;            // Mode actuel
    bool isActive;                  // Ecran actif ?
    uint32_t lastUpdate;            // Derniere mise a jour
    
    // ============================================
    // NAVIGATION DANS LA LISTE
    // ============================================
    ContactInfo_t contacts[MAX_CONTACTS_DISPLAY];  // Contacts charges
    uint8_t contactCount;           // Nombre total de contacts
    uint8_t selectedIndex;          // Index selectionne (0-based)
    uint8_t scrollOffset;           // Decalage de defilement
    uint8_t visibleCount;           // Nombre visible a l ecran
    
    // ============================================
    // CONTACT EN COURS D EDITION
    // ============================================
    char editName[32];              // Nom en edition
    char editNumber[16];            // Numero en edition
    uint8_t editNameIndex;          // Position curseur nom
    uint8_t editNumberIndex;        // Position curseur numero
    EditField_t editField;          // Champ en cours d edition
    bool editIsNew;                 // true = nouveau contact
    uint8_t editOriginalIndex;      // Index original (pour modification)
    
    // ============================================
    // RECHERCHE
    // ============================================
    char searchQuery[32];           // Texte de recherche
    uint8_t searchResults[MAX_CONTACTS_DISPLAY];  // Indices des resultats
    uint8_t searchCount;            // Nombre de resultats
    
    // ============================================
    // TOUCHE D AIDE
    // ============================================
    char lastKeyPressed;            // Derniere touche (pour navigation rapide)
    uint32_t lastKeyTime;           // Temps de la derniere touche
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Charge les contacts depuis le stockage
    void loadContacts(void);
    
    // Sauvegarde les contacts
    void saveContacts(void);
    
    // Trie les contacts par ordre alphabetique
    void sortContacts(void);
    
    // Dessine selon le mode
    void drawListMode(void);
    void drawViewMode(void);
    void drawEditMode(void);
    void drawDeleteConfirm(void);
    void drawSearchMode(void);
    
    // Dessine un element de la liste
    void drawContactItem(uint8_t index, uint8_t y, bool selected);
    
    // Dessine l en-tete
    void drawHeader(const char* title);
    
    // Dessine les instructions en bas
    void drawListInstructions(void);
    void drawViewInstructions(void);
    void drawEditInstructions(void);
    
    // Navigation rapide par premiere lettre
    void quickNavigate(char key);
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    ContactsScreen();
    void init(void);
    
    // ============================================
    // CYCLE DE VIE
    // ============================================
    void onEnter(void);
    void onExit(void);
    void update(void);
    void draw(void);
    
    // ============================================
    // GESTION DES TOUCHES
    // ============================================
    
    // Traite une touche
    // Retourne :
    //   0 = l ecran gere
    //   1 = lancer appel (numero dispo)
    //   2 = envoyer SMS
    //   255 = retour accueil
    uint8_t handleKeyPress(char key);
    
    // ============================================
    // ACTIONS SUR LES CONTACTS
    // ============================================
    
    // Ouvre le detail d un contact
    void viewContact(uint8_t index);
    
    // Commence l ajout d un contact
    void addContact(void);
    
    // Commence l edition d un contact
    void editContact(uint8_t index);
    
    // Supprime un contact
    void deleteContact(uint8_t index);
    
    // Bascule l etat favori
    void toggleFavorite(uint8_t index);
    
    // Appelle le contact selectionne
    const char* getSelectedNumber(void);
    const char* getSelectedName(void);
    
    // Recherche un contact
    void searchContacts(const char* query);
    
    // ============================================
    // INFORMATIONS
    // ============================================
    uint8_t getContactCount(void);
    bool isActiveScreen(void);
    ContactsMode_t getMode(void);
};

#endif // CONTACTS_SCREEN_H