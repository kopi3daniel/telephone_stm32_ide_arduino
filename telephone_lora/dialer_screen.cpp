

#include "dialer_screen.h"
#include "display.h"
#include <string.h>
#include <stdio.h>

extern Display display;
// extern PhoneContacts contacts;  // Module contacts (a implementer)

// ============================================
// CONSTRUCTEUR
// ============================================
DialerScreen::DialerScreen() {
    state = DIALER_STATE_IDLE;
    isActive = false;
    lastUpdate = 0;
    
    memset(dialNumber, 0, sizeof(dialNumber));
    dialIndex = 0;
    cursorVisible = true;
    lastCursorToggle = 0;
    
    memset(suggestions, 0, sizeof(suggestions));
    suggestionCount = 0;
    selectedSuggestion = 0;
    
    memset(recognizedName, 0, sizeof(recognizedName));
    isRecognized = false;
}

void DialerScreen::init(void) {
    state = DIALER_STATE_IDLE;
    isActive = false;
    reset();
}

// ============================================
// CYCLE DE VIE
// ============================================
void DialerScreen::onEnter(void) {
    isActive = true;
    state = DIALER_STATE_IDLE;
    lastUpdate = millis();
    reset();
    draw();
}

void DialerScreen::onExit(void) {
    isActive = false;
}

void DialerScreen::update(void) {
    if (!isActive) return;
    
    uint32_t now = millis();
    
    // Faire clignoter le curseur toutes les 500ms
    if (now - lastCursorToggle > 500) {
        cursorVisible = !cursorVisible;
        lastCursorToggle = now;
        draw();
    }
}

// ============================================
// DESSIN
// ============================================
void DialerScreen::draw(void) {
    display.clear();
    
    // Barre d etat
    display.drawStatusBar();
    
    // Titre
    display.drawCenteredText(10, "Composer", 1);
    display.drawLine(0, 18, SCREEN_WIDTH, 18);
    
    // Zone du numero
    drawNumber();
    
    // Ligne de separation
    display.drawLine(0, 38, SCREEN_WIDTH, 38);
    
    // Suggestions ou nom reconnu
    if (isRecognized) {
        drawRecognizedName();
    } else if (suggestionCount > 0) {
        drawSuggestions();
    }
    
    // Instructions en bas
    drawInstructions();
    
    display.refresh();
}

void DialerScreen::drawNumber(void) {
    char displayNumber[20];
    
    // Formater le numero avec des espaces
    if (dialIndex > 0) {
        formatNumberForDisplay(displayNumber, dialNumber);
    } else {
        strcpy(displayNumber, "");
    }
    
    // Afficher le numero en grand
    display.drawText(5, 22, displayNumber, 2);
    
    // Curseur clignotant apres le numero
    if (cursorVisible) {
        uint8_t cursorX = 5 + (strlen(displayNumber) * 12);
        if (cursorX < SCREEN_WIDTH - 5) {
            display.drawLine(cursorX, 24, cursorX, 36);
        }
    }
}

void DialerScreen::drawSuggestions(void) {
    // Afficher jusqu a 5 suggestions
    for (uint8_t i = 0; i < suggestionCount && i < 5; i++) {
        uint8_t y = 40 + (i * 9);
        
        // Surligner la suggestion selectionnee
        if (i == selectedSuggestion) {
            display.drawRect(0, y, SCREEN_WIDTH, 9, true);
            display.drawInvertedText(2, y, suggestions[i].name, 1);
            
            char numStr[18];
            snprintf(numStr, sizeof(numStr), "%s", suggestions[i].number);
            uint8_t numX = SCREEN_WIDTH - (strlen(numStr) * 6) - 2;
            display.drawInvertedText(numX, y, numStr, 1);
        } else {
            display.drawText(2, y, suggestions[i].name, 1);
            
            char numStr[18];
            snprintf(numStr, sizeof(numStr), "%s", suggestions[i].number);
            uint8_t numX = SCREEN_WIDTH - (strlen(numStr) * 6) - 2;
            display.drawText(numX, y, numStr, 1);
        }
        
        // Etoile pour les favoris
        if (suggestions[i].isFavorite) {
            display.drawText(SCREEN_WIDTH - 10, y, "*", 1);
        }
    }
}

void DialerScreen::drawRecognizedName(void) {
    // Afficher le nom du contact reconnu
    display.drawCenteredText(42, recognizedName, 1);
    
    // Afficher "Contact enregistre"
    display.drawCenteredText(52, "Contact enregistre", 1);
}

void DialerScreen::drawInstructions(void) {
    display.drawText(0, SCREEN_HEIGHT - 8, "C:Effacer", 1);
    display.drawCenteredText(SCREEN_HEIGHT - 8, "VERT:Appeler", 1);
    display.drawText(SCREEN_WIDTH - 60, SCREEN_HEIGHT - 8, "ROUGE:Retour", 1);
}

// ============================================
// GESTION DES TOUCHES
// ============================================
// uint8_t DialerScreen::handleKeyPress(char key) {
//     if (!isActive) return 0;
    
//     switch (key) {
        
//         // ============================================
//         // CHIFFRES 0-9
//         // ============================================
//         case '0': case '1': case '2': case '3': case '4':
//         case '5': case '6': case '7': case '8': case '9':
//             if (addDigit(key)) {
//                 state = DIALER_STATE_TYPING;
//                 findSuggestions();
//                 checkRecognizedNumber();
//                 draw();
//             }
//             return 0;
        
//         // ============================================
//         // ETOILE (*) = + pour numeros internationaux
//         // ============================================
//         case '*':
//             if (dialIndex == 0) {
//                 addDigit('+');
//             } else {
//                 addDigit('*');
//             }
//             draw();
//             return 0;
        
//         // ============================================
//         // DIESE (#) = pause ou caractere special
//         // ============================================
//         case '#':
//             addDigit('#');
//             draw();
//             return 0;
        
//         // ============================================
//         // BOUTON C : EFFACER
//         // ============================================
//         case 'C':
//             deleteDigit();
//             findSuggestions();
//             checkRecognizedNumber();
//             draw();
//             return 0;
        
//         // ============================================
//         // NAVIGATION HAUT/BAS : selection suggestions
//         // ============================================
//         case '2':  // Haut
//             if (suggestionCount > 1) {
//                 if (selectedSuggestion > 0) {
//                     selectedSuggestion--;
//                 } else {
//                     selectedSuggestion = suggestionCount - 1;
//                 }
//                 draw();
//             }
//             return 0;
        
//         case '8':  // Bas
//             if (suggestionCount > 1) {
//                 if (selectedSuggestion < suggestionCount - 1) {
//                     selectedSuggestion++;
//                 } else {
//                     selectedSuggestion = 0;
//                 }
//                 draw();
//             }
//             return 0;
        
//         // ============================================
//         // NAVIGATION GAUCHE/DROITE : deplacer curseur
//         // ============================================
//         case '4':  // Gauche
//             if (dialIndex > 0) {
//                 dialIndex--;
//                 draw();
//             }
//             return 0;
        
//         case '6':  // Droite
//             if (dialIndex < strlen(dialNumber)) {
//                 dialIndex++;
//                 draw();
//             }
//             return 0;
        
//         // ============================================
//         // BOUTON VERT (A) : APPELER
//         // ============================================
//         case 'A':
//             if (suggestionCount > 0 && selectedSuggestion < suggestionCount) {
//                 strncpy(dialNumber, suggestions[selectedSuggestion].number, 15);
//                 dialIndex = strlen(dialNumber);
//                 state = DIALER_STATE_CALLING;
//                 return 1;
//             }
            
//             if (dialIndex > 0) {
//                 state = DIALER_STATE_CALLING;
//                 return 1;
//             }
            
//             return 0;
        
//         // ============================================
//         // BOUTON ROUGE (B) : RETOUR
//         // ============================================
//         case 'B':
//             state = DIALER_STATE_IDLE;
//             return 255;
        
//         default:
//             return 0;
//     }
// }


uint8_t DialerScreen::handleKeyPress(char key) {
    if (!isActive) return 0;
    
    switch (key) {
        
        // ============================================
        // CHIFFRES 0-9
        // ============================================
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if (addDigit(key)) {
                state = DIALER_STATE_TYPING;
                findSuggestions();
                checkRecognizedNumber();
                draw();
            }
            return 0;
        
        // ============================================
        // ETOILE (*)
        // ============================================
        case '*':
            if (dialIndex == 0) {
                addDigit('+');
            } else {
                addDigit('*');
            }
            draw();
            return 0;
        
        // ============================================
        // DIESE (#)
        // ============================================
        case '#':
            addDigit('#');
            draw();
            return 0;
        
        // ============================================
        // BOUTON C : EFFACER
        // ============================================
        case 'C':
            deleteDigit();
            findSuggestions();
            checkRecognizedNumber();
            draw();
            return 0;
        
        // ============================================
        // BOUTON VERT (A) : APPELER
        // ============================================
        case 'A':
            if (suggestionCount > 0 && selectedSuggestion < suggestionCount) {
                strncpy(dialNumber, suggestions[selectedSuggestion].number, 15);
                dialIndex = strlen(dialNumber);
                state = DIALER_STATE_CALLING;
                return 1;
            }
            
            if (dialIndex > 0) {
                state = DIALER_STATE_CALLING;
                return 1;
            }
            return 0;
        
        // ============================================
        // BOUTON ROUGE (B) : RETOUR
        // ============================================
        case 'B':
            state = DIALER_STATE_IDLE;
            return 255;
        
        // ============================================
        // TOUCHES DE NAVIGATION (D, E, F, G, H, I, J, K, L)
        // ============================================
        case 'D':  // Haut - suggestions precedentes
            if (suggestionCount > 1) {
                if (selectedSuggestion > 0) {
                    selectedSuggestion--;
                } else {
                    selectedSuggestion = suggestionCount - 1;
                }
                draw();
            }
            return 0;
        
        case 'E':  // Bas - suggestions suivantes
            if (suggestionCount > 1) {
                if (selectedSuggestion < suggestionCount - 1) {
                    selectedSuggestion++;
                } else {
                    selectedSuggestion = 0;
                }
                draw();
            }
            return 0;
        
        case 'F':  // Gauche - deplacer curseur
            if (dialIndex > 0) {
                dialIndex--;
                draw();
            }
            return 0;
        
        case 'G':  // Droite - deplacer curseur
            if (dialIndex < (uint8_t)strlen(dialNumber)) {
                dialIndex++;
                draw();
            }
            return 0;
        
        default:
            return 0;
    }
}


// ============================================
// AJOUTER UN CHIFFRE
// ============================================
bool DialerScreen::addDigit(char digit) {
    if (dialIndex >= 15) {
        return false;
    }
    
    if (dialIndex < (uint8_t)strlen(dialNumber)) {
        for (int i = strlen(dialNumber); i > (int)dialIndex; i--) {
            dialNumber[i] = dialNumber[i - 1];
        }
    }
    
    dialNumber[dialIndex] = digit;
    dialIndex++;
    dialNumber[dialIndex] = '\0';
    
    return true;
}

// ============================================
// EFFACER LE DERNIER CHIFFRE
// ============================================
void DialerScreen::deleteDigit(void) {
    if (dialIndex > 0) {
        for (uint8_t i = dialIndex - 1; i < strlen(dialNumber); i++) {
            dialNumber[i] = dialNumber[i + 1];
        }
        dialIndex--;
        
        if (dialIndex == 0) {
            state = DIALER_STATE_IDLE;
            suggestionCount = 0;
            isRecognized = false;
        }
    }
}

// ============================================
// EFFACER TOUT
// ============================================
void DialerScreen::clearAll(void) {
    memset(dialNumber, 0, sizeof(dialNumber));
    dialIndex = 0;
    state = DIALER_STATE_IDLE;
    suggestionCount = 0;
    selectedSuggestion = 0;
    isRecognized = false;
}

// ============================================
// RECHERCHE DE SUGGESTIONS
// ============================================
void DialerScreen::findSuggestions(void) {
    suggestionCount = 0;
    selectedSuggestion = 0;
    
    if (dialIndex == 0) return;
    
    // TODO: integrer avec le module contacts
    // Simulation pour test
    if (strncmp(dialNumber, "06", 2) == 0) {
        strcpy(suggestions[0].name, "Alice");
        strcpy(suggestions[0].number, "0601020304");
        suggestions[0].isFavorite = true;
        
        strcpy(suggestions[1].name, "Bob");
        strcpy(suggestions[1].number, "0605060708");
        suggestions[1].isFavorite = false;
        
        suggestionCount = 2;
    }
}

// ============================================
// VERIFIER SI LE NUMERO EST RECONNU
// ============================================
void DialerScreen::checkRecognizedNumber(void) {
    isRecognized = false;
    memset(recognizedName, 0, sizeof(recognizedName));
    
    if (dialIndex == 0) return;
    
    // TODO: integrer avec le module contacts
    if (strcmp(dialNumber, "0601020304") == 0) {
        strcpy(recognizedName, "Alice");
        isRecognized = true;
    }
}

// ============================================
// FORMATAGE DU NUMERO POUR AFFICHAGE
// ============================================
void DialerScreen::formatNumberForDisplay(char* output, const char* input) {
    uint8_t inLen = strlen(input);
    uint8_t outIdx = 0;
    
    for (uint8_t i = 0; i < inLen; i++) {
        if (i > 0 && i % 2 == 0 && input[0] != '+') {
            output[outIdx++] = ' ';
        }
        output[outIdx++] = input[i];
    }
    
    output[outIdx] = '\0';
}

// ============================================
// INFORMATIONS
// ============================================
const char* DialerScreen::getNumber(void) {
    return dialNumber;
}

uint8_t DialerScreen::getNumberLength(void) {
    return dialIndex;
}

bool DialerScreen::hasNumber(void) {
    return (dialIndex > 0);
}

DialerState_t DialerScreen::getState(void) {
    return state;
}

bool DialerScreen::isActiveScreen(void) {
    return isActive;
}

// ============================================
// ACTIONS EXTERNES
// ============================================
void DialerScreen::showError(const char* message) {
    state = DIALER_STATE_ERROR;
    
    display.drawCenteredText(45, message, 1);
    display.refresh();
    delay(1500);
    
    state = DIALER_STATE_TYPING;
    draw();
}

void DialerScreen::reset(void) {
    clearAll();
    draw();
}

void DialerScreen::setNumber(const char* number) {
    if (number != NULL) {
        strncpy(dialNumber, number, 15);
        dialNumber[15] = '\0';
        dialIndex = strlen(dialNumber);
        checkRecognizedNumber();
        draw();
    }
}

// ============================================
// FIN DU FICHIER dialer_screen.cpp
// ============================================