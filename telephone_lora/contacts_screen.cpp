/**
 * ---------------------------------------------------------------------------
 * contacts_screen.cpp - Implementation de l ecran contacts
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "contacts_screen.h"
#include "display.h"
#include "keyboard.h"
#include <string.h>
#include <stdio.h>

extern Display display;
extern Keyboard keyboard;

// ============================================
// CONSTRUCTEUR
// ============================================
ContactsScreen::ContactsScreen() {
    mode = CONTACTS_MODE_LIST;
    isActive = false;
    lastUpdate = 0;
    
    contactCount = 0;
    selectedIndex = 0;
    scrollOffset = 0;
    visibleCount = 0;
    
    memset(contacts, 0, sizeof(contacts));
    memset(editName, 0, sizeof(editName));
    memset(editNumber, 0, sizeof(editNumber));
    editNameIndex = 0;
    editNumberIndex = 0;
    editField = EDIT_FIELD_NAME;
    editIsNew = false;
    editOriginalIndex = 0;
    
    memset(searchQuery, 0, sizeof(searchQuery));
    memset(searchResults, 0, sizeof(searchResults));
    searchCount = 0;
    
    lastKeyPressed = 0;
    lastKeyTime = 0;
}

void ContactsScreen::init(void) {
    isActive = false;
    mode = CONTACTS_MODE_LIST;
    selectedIndex = 0;
    scrollOffset = 0;
}

// ============================================
// CYCLE DE VIE
// ============================================
void ContactsScreen::onEnter(void) {
    isActive = true;
    loadContacts();
    sortContacts();
    draw();
}

void ContactsScreen::onExit(void) {
    isActive = false;
}

void ContactsScreen::update(void) {
    if (!isActive) return;
}

// ============================================
// DESSIN PRINCIPAL
// ============================================
void ContactsScreen::draw(void) {
    display.clear();
    
    switch (mode) {
        case CONTACTS_MODE_LIST:
            drawListMode();
            break;
        case CONTACTS_MODE_VIEW:
            drawViewMode();
            break;
        case CONTACTS_MODE_EDIT:
        case CONTACTS_MODE_ADD:
            drawEditMode();
            break;
        case CONTACTS_MODE_DELETE:
            drawDeleteConfirm();
            break;
        case CONTACTS_MODE_SEARCH:
            drawSearchMode();
            break;
    }
    
    display.refresh();
}

// ============================================
// MODE LISTE
// ============================================
void ContactsScreen::drawListMode(void) {
    drawHeader("Contacts");
    
    visibleCount = (contactCount < CONTACTS_PER_PAGE) ? 
                   contactCount : CONTACTS_PER_PAGE;
    
    if (selectedIndex >= scrollOffset + CONTACTS_PER_PAGE) {
        scrollOffset = selectedIndex - CONTACTS_PER_PAGE + 1;
    }
    if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
    }
    
    for (uint8_t i = 0; i < visibleCount; i++) {
        uint8_t contactIdx = scrollOffset + i;
        if (contactIdx >= contactCount) break;
        
        uint8_t y = 12 + (i * 10);
        bool isSelected = (contactIdx == selectedIndex);
        drawContactItem(contactIdx, y, isSelected);
    }
    
    if (contactCount > CONTACTS_PER_PAGE) {
        uint8_t scrollBarH = (CONTACTS_PER_PAGE * 54) / contactCount;
        uint8_t scrollBarY = 12 + (scrollOffset * 54) / contactCount;
        display.drawRect(SCREEN_WIDTH - 3, scrollBarY, 2, scrollBarH, true);
    }
    
    drawListInstructions();
}

void ContactsScreen::drawContactItem(uint8_t index, uint8_t y, bool selected) {
    if (index >= contactCount) return;
    
    if (selected) {
        display.drawRect(0, y - 1, SCREEN_WIDTH, 10, true);
    }
    
    if (contacts[index].isFavorite) {
        display.drawText(2, y, "*", 1);
    }
    
    char displayText[40];
    if (selected) {
        snprintf(displayText, sizeof(displayText), "%s", contacts[index].name);
        display.drawInvertedText(12, y, displayText, 1);
    } else {
        snprintf(displayText, sizeof(displayText), "%s", contacts[index].name);
        display.drawText(12, y, displayText, 1);
    }
    
    if (selected) {
        display.drawInvertedText(SCREEN_WIDTH - 80, y, contacts[index].number, 1);
    } else {
        display.drawText(SCREEN_WIDTH - 80, y, contacts[index].number, 1);
    }
}

void ContactsScreen::drawHeader(const char* title) {
    display.drawCenteredText(0, title, 1);
    display.drawLine(0, 9, SCREEN_WIDTH, 9);
    
    char countStr[10];
    snprintf(countStr, sizeof(countStr), "%d", contactCount);
    display.drawText(SCREEN_WIDTH - 20, 0, countStr, 1);
}

void ContactsScreen::drawListInstructions(void) {
    display.drawText(0, SCREEN_HEIGHT - 10, "VERT:Appel", 1);
    display.drawText(50, SCREEN_HEIGHT - 10, "C:Voir", 1);
    display.drawText(90, SCREEN_HEIGHT - 10, "D:Ajouter", 1);
}

// ============================================
// MODE VUE (DETAIL)
// ============================================
void ContactsScreen::drawViewMode(void) {
    if (selectedIndex >= contactCount) return;
    
    ContactInfo_t* contact = &contacts[selectedIndex];
    
    drawHeader(contact->name);
    
    if (contact->isFavorite) {
        display.drawText(2, 15, "* Favori", 1);
    }
    
    display.drawText(2, 25, contact->name, 1);
    display.drawText(2, 35, contact->number, 2);
    
    display.drawLine(0, 48, SCREEN_WIDTH, 48);
    
    drawViewInstructions();
}

void ContactsScreen::drawViewInstructions(void) {
    display.drawText(0, SCREEN_HEIGHT - 10, "VERT:Appeler", 1);
    display.drawText(55, SCREEN_HEIGHT - 10, "C:Modifier", 1);
    display.drawText(105, SCREEN_HEIGHT - 10, "B:Retour", 1);
}

// ============================================
// MODE EDITION (AJOUT/MODIFICATION)
// ============================================
void ContactsScreen::drawEditMode(void) {
    const char* title = editIsNew ? "Nouveau contact" : "Modifier contact";
    drawHeader(title);
    
    // Champ Nom
    display.drawText(2, 14, "Nom:", 1);
    display.drawRect(30, 12, 96, 10, false);
    display.drawText(32, 13, editName, 1);
    
    if (editField == EDIT_FIELD_NAME) {
        uint8_t cursorX = 32 + (editNameIndex * 6);
        display.drawLine(cursorX, 13, cursorX, 20);
    }
    
    // Champ Numero
    display.drawText(2, 28, "Num:", 1);
    display.drawRect(30, 26, 96, 10, false);
    display.drawText(32, 27, editNumber, 1);
    
    if (editField == EDIT_FIELD_NUMBER) {
        uint8_t cursorX = 32 + (editNumberIndex * 6);
        display.drawLine(cursorX, 27, cursorX, 34);
    }
    
    drawEditInstructions();
}

void ContactsScreen::drawEditInstructions(void) {
    display.drawText(0, 42, "Mode:", 1);
    
    const char* modeStr = keyboard.getModeString();
    display.drawText(30, 42, modeStr, 1);
    
    display.drawText(0, SCREEN_HEIGHT - 10, "VERT:Enregistrer", 1);
    display.drawText(70, SCREEN_HEIGHT - 10, "B:Annuler", 1);
    display.drawText(105, SCREEN_HEIGHT - 10, "J:Mode", 1);
}

// ============================================
// MODE CONFIRMATION SUPPRESSION
// ============================================
void ContactsScreen::drawDeleteConfirm(void) {
    if (selectedIndex >= contactCount) return;
    
    drawHeader("Supprimer ?");
    
    display.drawCenteredText(25, contacts[selectedIndex].name, 1);
    display.drawCenteredText(35, contacts[selectedIndex].number, 1);
    
    // display.drawText(10, 50, "VERT: Oui");
    // display.drawText(70, 50, "ROUGE: Non");

    // Correction : ajouter le 4eme parametre (taille de police = 1)
    display.drawText(10, 50, "VERT: Oui", 1);      // <-- ajouter , 1
    display.drawText(70, 50, "ROUGE: Non", 1);      // <-- ajouter , 1
}

// ============================================
// MODE RECHERCHE
// ============================================
void ContactsScreen::drawSearchMode(void) {
    drawHeader("Rechercher");
    
    display.drawRect(2, 12, SCREEN_WIDTH - 4, 10, false);
    display.drawText(4, 13, searchQuery, 1);
    
    for (uint8_t i = 0; i < searchCount && i < 5; i++) {
        uint8_t y = 26 + (i * 10);
        uint8_t contactIdx = searchResults[i];
        
        bool isSelected = (i == selectedIndex);
        drawContactItem(contactIdx, y, isSelected);
    }
}

// ============================================
// GESTION DES TOUCHES
// ============================================
uint8_t ContactsScreen::handleKeyPress(char key) {
    if (!isActive) return 0;
    
    switch (mode) {
        
        // ============================================
        // MODE LISTE
        // ============================================
        case CONTACTS_MODE_LIST:
            switch (key) {
                case '2':  // Haut
                    if (selectedIndex > 0) {
                        selectedIndex--;
                        draw();
                    }
                    return 0;
                    
                case '8':  // Bas
                    if (selectedIndex < contactCount - 1) {
                        selectedIndex++;
                        draw();
                    }
                    return 0;
                    
                case 'A':  // VERT = Appeler
                    return 1;
                    
                case 'C':  // Voir details
                    viewContact(selectedIndex);
                    return 0;
                    
                case 'D':  // Ajouter
                    addContact();
                    return 0;
                    
                case 'B':  // ROUGE = Retour
                    return 255;
                    
                case '*':  // Favori
                    toggleFavorite(selectedIndex);
                    return 0;
                    
                default:
                    if ((key >= '2' && key <= '9') || key == '0') {
                        quickNavigate(key);
                        draw();
                    }
                    return 0;
            }
        
        // ============================================
        // MODE VUE
        // ============================================
        case CONTACTS_MODE_VIEW:
            switch (key) {
                case 'A':  // Appeler
                    return 1;
                case 'C':  // Modifier
                    editContact(selectedIndex);
                    return 0;
                case 'D':  // Supprimer
                    mode = CONTACTS_MODE_DELETE;
                    draw();
                    return 0;
                case 'B':  // Retour liste
                    mode = CONTACTS_MODE_LIST;
                    draw();
                    return 0;
                case '*':  // Favori
                    toggleFavorite(selectedIndex);
                    draw();
                    return 0;
                default:
                    return 0;
            }
        
        // ============================================
        // MODE EDITION
        // ============================================
        case CONTACTS_MODE_EDIT:
        case CONTACTS_MODE_ADD:
            switch (key) {
                case 'J':  // Changer mode saisie
                    keyboard.toggleMode();
                    draw();
                    return 0;
                    
                case 'I':  // Shift
                    keyboard.toggleShift();
                    return 0;
                    
                case '*':  // Basculer champ
                    editField = (editField == EDIT_FIELD_NAME) ? 
                               EDIT_FIELD_NUMBER : EDIT_FIELD_NAME;
                    draw();
                    return 0;
                    
                case 'C':  // Effacer
                    if (editField == EDIT_FIELD_NAME && editNameIndex > 0) {
                        editNameIndex--;
                        editName[editNameIndex] = '\0';
                    } else if (editField == EDIT_FIELD_NUMBER && editNumberIndex > 0) {
                        editNumberIndex--;
                        editNumber[editNumberIndex] = '\0';
                    }
                    draw();
                    return 0;
                    
                case 'A':  // Enregistrer
                    mode = CONTACTS_MODE_LIST;
                    loadContacts();
                    draw();
                    return 0;
                    
                case 'B':  // Annuler
                    mode = CONTACTS_MODE_LIST;
                    draw();
                    return 0;
                    
                default:
                    if (key >= '0' && key <= '9') {
                        char c = keyboard.multiTapInput(key);
                        if (c != 0) {
                            if (editField == EDIT_FIELD_NAME && editNameIndex < 31) {
                                editName[editNameIndex++] = c;
                                editName[editNameIndex] = '\0';
                            } else if (editField == EDIT_FIELD_NUMBER && editNumberIndex < 15) {
                                editNumber[editNumberIndex++] = c;
                                editNumber[editNumberIndex] = '\0';
                            }
                            draw();
                        }
                    }
                    return 0;
            }
        
        // ============================================
        // MODE SUPPRESSION
        // ============================================
        case CONTACTS_MODE_DELETE:
            switch (key) {
                case 'A':  // Confirmer
                    deleteContact(selectedIndex);
                    mode = CONTACTS_MODE_LIST;
                    loadContacts();
                    draw();
                    return 0;
                case 'B':  // Annuler
                    mode = CONTACTS_MODE_VIEW;
                    draw();
                    return 0;
                default:
                    return 0;
            }
        
        default:
            return 0;
    }
}

// ============================================
// ACTIONS SUR LES CONTACTS
// ============================================
void ContactsScreen::loadContacts(void) {
    // TODO: Charger depuis le module storage
    // Pour l instant, contacts de test
    contactCount = 3;
    
    strcpy(contacts[0].name, "Alice");
    strcpy(contacts[0].number, "0001");
    contacts[0].isFavorite = true;
    contacts[0].index = 0;
    
    strcpy(contacts[1].name, "Bob");
    strcpy(contacts[1].number, "0002");
    contacts[1].isFavorite = false;
    contacts[1].index = 1;
    
    strcpy(contacts[2].name, "Charlie");
    strcpy(contacts[2].number, "0003");
    contacts[2].isFavorite = false;
    contacts[2].index = 2;
}

void ContactsScreen::sortContacts(void) {
    for (uint8_t i = 0; i < contactCount - 1; i++) {
        for (uint8_t j = 0; j < contactCount - i - 1; j++) {
            if (strcmp(contacts[j].name, contacts[j + 1].name) > 0) {
                ContactInfo_t temp = contacts[j];
                contacts[j] = contacts[j + 1];
                contacts[j + 1] = temp;
            }
        }
    }
}

void ContactsScreen::viewContact(uint8_t index) {
    if (index < contactCount) {
        selectedIndex = index;
        mode = CONTACTS_MODE_VIEW;
        draw();
    }
}

void ContactsScreen::addContact(void) {
    mode = CONTACTS_MODE_ADD;
    editIsNew = true;
    editField = EDIT_FIELD_NAME;
    memset(editName, 0, sizeof(editName));
    memset(editNumber, 0, sizeof(editNumber));
    editNameIndex = 0;
    editNumberIndex = 0;
    draw();
}

void ContactsScreen::editContact(uint8_t index) {
    if (index >= contactCount) return;
    
    mode = CONTACTS_MODE_EDIT;
    editIsNew = false;
    editOriginalIndex = index;
    editField = EDIT_FIELD_NAME;
    
    strncpy(editName, contacts[index].name, 31);
    strncpy(editNumber, contacts[index].number, 15);
    editNameIndex = strlen(editName);
    editNumberIndex = strlen(editNumber);
    
    draw();
}

void ContactsScreen::deleteContact(uint8_t index) {
    if (index >= contactCount) return;
    
    for (uint8_t i = index; i < contactCount - 1; i++) {
        contacts[i] = contacts[i + 1];
    }
    contactCount--;
    
    if (selectedIndex >= contactCount && contactCount > 0) {
        selectedIndex = contactCount - 1;
    }
}

void ContactsScreen::toggleFavorite(uint8_t index) {
    if (index < contactCount) {
        contacts[index].isFavorite = !contacts[index].isFavorite;
    }
}

const char* ContactsScreen::getSelectedNumber(void) {
    if (selectedIndex < contactCount) {
        return contacts[selectedIndex].number;
    }
    return NULL;
}

const char* ContactsScreen::getSelectedName(void) {
    if (selectedIndex < contactCount) {
        return contacts[selectedIndex].name;
    }
    return NULL;
}

// ============================================
// NAVIGATION RAPIDE PAR LETTRE
// ============================================
void ContactsScreen::quickNavigate(char key) {
    const char* t9Letters[] = {
        " ",     // 0
        ".",     // 1
        "a",     // 2
        "d",     // 3
        "g",     // 4
        "j",     // 5
        "m",     // 6
        "p",     // 7
        "t",     // 8
        "w"      // 9
    };
    
    if (key >= '2' && key <= '9') {
        uint8_t digit = key - '0';
        char target = t9Letters[digit][0];
        
        for (uint8_t i = 0; i < contactCount; i++) {
            if (contacts[i].name[0] >= target) {
                selectedIndex = i;
                return;
            }
        }
    }
}

// ============================================
// INFORMATIONS
// ============================================
uint8_t ContactsScreen::getContactCount(void) { return contactCount; }
bool ContactsScreen::isActiveScreen(void) { return isActive; }
ContactsMode_t ContactsScreen::getMode(void) { return mode; }

// ============================================
// FIN DU FICHIER contacts_screen.cpp
// ============================================