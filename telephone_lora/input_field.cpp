

#include "input_field.h"
#include "display.h"
#include <string.h>
#include <stdio.h>

extern Display display;

// ============================================
// CONSTRUCTEUR
// ============================================
InputField::InputField() {
    memset(text, 0, sizeof(text));
    textLength = 0;
    maxLength = INPUT_MAX_LENGTH;
    cursorPos = 0;
    
    fieldX = 0;
    fieldY = 0;
    fieldWidth = SCREEN_WIDTH;
    fieldHeight = 12;
    
    cursorVisible = true;
    lastCursorToggle = 0;
    needsRedraw = true;
    
    previewChar = 0;
    previewStartTime = 0;
    previewActive = false;
    
    memset(modeIndicator, 0, sizeof(modeIndicator));
    strcpy(modeIndicator, "abc");
    
    showBorder = true;
    showCounter = true;
    showMode = true;
    isActive = false;
    
    scrollOffset = 0;
    visibleChars = 20;
}

void InputField::init(void) {
    clear();
    cursorPos = 0;
    scrollOffset = 0;
    needsRedraw = true;
}

// ============================================
// CONFIGURATION
// ============================================
void InputField::setBounds(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    fieldX = x;
    fieldY = y;
    fieldWidth = width;
    fieldHeight = height;
    visibleChars = (width - 4) / 6;
    needsRedraw = true;
}

void InputField::setMaxLength(uint8_t max) {
    maxLength = (max > INPUT_MAX_LENGTH) ? INPUT_MAX_LENGTH : max;
}

void InputField::showFieldBorder(bool show) {
    showBorder = show;
    needsRedraw = true;
}

void InputField::showCharCounter(bool show) {
    showCounter = show;
    needsRedraw = true;
}

void InputField::showModeDisplay(bool show) {
    showMode = show;
    needsRedraw = true;
}

void InputField::setMode(const char* mode) {
    if (mode) {
        strncpy(modeIndicator, mode, 3);
        modeIndicator[3] = '\0';
        needsRedraw = true;
    }
}

// ============================================
// EDITION DU TEXTE
// ============================================

void InputField::addChar(char c) {
    if (textLength >= maxLength) return;
    
    if (cursorPos < textLength) {
        for (int i = textLength; i > (int)cursorPos; i--) {
            text[i] = text[i - 1];
        }
    }
    
    text[cursorPos] = c;
    cursorPos++;
    textLength++;
    text[textLength] = '\0';
    
    adjustScroll();
    needsRedraw = true;
}

void InputField::deleteChar(void) {
    if (cursorPos > 0) {
        cursorPos--;
        for (uint8_t i = cursorPos; i < textLength - 1; i++) {
            text[i] = text[i + 1];
        }
        textLength--;
        text[textLength] = '\0';
        adjustScroll();
        needsRedraw = true;
    }
}

void InputField::deleteCharForward(void) {
    if (cursorPos < textLength) {
        for (uint8_t i = cursorPos; i < textLength - 1; i++) {
            text[i] = text[i + 1];
        }
        textLength--;
        text[textLength] = '\0';
        needsRedraw = true;
    }
}

void InputField::clear(void) {
    memset(text, 0, sizeof(text));
    textLength = 0;
    cursorPos = 0;
    scrollOffset = 0;
    previewActive = false;
    previewChar = 0;
    needsRedraw = true;
}

void InputField::cursorLeft(void) {
    if (cursorPos > 0) {
        cursorPos--;
        adjustScroll();
        needsRedraw = true;
    }
}

void InputField::cursorRight(void) {
    if (cursorPos < textLength) {
        cursorPos++;
        adjustScroll();
        needsRedraw = true;
    }
}

void InputField::cursorHome(void) {
    cursorPos = 0;
    scrollOffset = 0;
    needsRedraw = true;
}

void InputField::cursorEnd(void) {
    cursorPos = textLength;
    adjustScroll();
    needsRedraw = true;
}

// ============================================
// PREVISUALISATION MULTI-TAP
// ============================================
void InputField::showPreview(char c) {
    previewChar = c;
    previewStartTime = millis();
    previewActive = true;
    needsRedraw = true;
}

void InputField::confirmPreview(void) {
    if (previewActive) {
        addChar(previewChar);
        previewActive = false;
        previewChar = 0;
        needsRedraw = true;
    }
}

void InputField::cancelPreview(void) {
    previewActive = false;
    previewChar = 0;
    needsRedraw = true;
}

// ============================================
// CONTENU
// ============================================
const char* InputField::getText(void) { return text; }
uint8_t InputField::getLength(void) { return textLength; }
uint8_t InputField::getRemainingChars(void) { return maxLength - textLength; }

void InputField::setText(const char* newText) {
    if (newText) {
        strncpy(text, newText, maxLength);
        text[maxLength] = '\0';
        textLength = strlen(text);
        cursorPos = textLength;
        adjustScroll();
        needsRedraw = true;
    }
}

// ============================================
// ETAT
// ============================================
void InputField::setActive(bool active) {
    isActive = active;
    if (active) {
        cursorVisible = true;
        lastCursorToggle = millis();
    }
    needsRedraw = true;
}

bool InputField::isActiveField(void) { return isActive; }
bool InputField::isEmpty(void) { return (textLength == 0); }
bool InputField::isFull(void) { return (textLength >= maxLength); }

// ============================================
// MISE A JOUR
// ============================================
void InputField::update(void) {
    if (!isActive) return;
    
    uint32_t now = millis();
    
    // Faire clignoter le curseur
    if (now - lastCursorToggle > INPUT_CURSOR_BLINK_MS) {
        cursorVisible = !cursorVisible;
        lastCursorToggle = now;
        needsRedraw = true;
    }
    
    // Verifier le timeout de previsualisation
    if (previewActive && (now - previewStartTime > INPUT_PREVIEW_TIMEOUT_MS)) {
        confirmPreview();
    }
    
    if (needsRedraw) {
        draw();
        needsRedraw = false;
    }
}

// ============================================
// DESSIN
// ============================================
void InputField::draw(void) {
    // Bordure
    if (showBorder) {
        display.drawRect(fieldX, fieldY, fieldWidth, fieldHeight, false);
    }
    
    // Texte visible
    drawVisibleText();
    
    // Curseur
    if (isActive && cursorVisible) {
        drawCursor();
    }
    
    // Compteur de caracteres
    if (showCounter) {
        drawCounter();
    }
    
    // Indicateur de mode
    if (showMode) {
        drawModeIndicator();
    }
}

void InputField::drawVisibleText(void) {
    uint8_t textX = fieldX + 2;
    uint8_t textY = fieldY + 2;
    
    for (uint8_t i = scrollOffset; i < textLength && i < scrollOffset + visibleChars; i++) {
        uint8_t drawX = textX + ((i - scrollOffset) * 6);
        
        if (i == cursorPos - 1 && previewActive) {
            char previewStr[2] = {previewChar, '\0'};
            display.drawText(drawX, textY, previewStr, 1);
        } else {
            char charStr[2] = {text[i], '\0'};
            display.drawText(drawX, textY, charStr, 1);
        }
    }
}

void InputField::drawCursor(void) {
    int16_t cursorX = fieldX + 2 + ((int16_t)(cursorPos - scrollOffset) * 6);
    
    if (cursorX >= fieldX && cursorX < fieldX + fieldWidth - 2) {
        display.drawLine(cursorX, fieldY + 2, cursorX, fieldY + fieldHeight - 3);
        
        if (previewActive) {
            display.drawLine(cursorX, fieldY + fieldHeight - 2, 
                           cursorX + 5, fieldY + fieldHeight - 2);
        }
    }
}

void InputField::drawCounter(void) {
    char counter[10];
    snprintf(counter, sizeof(counter), "%d/%d", textLength, maxLength);
    
    uint8_t counterX = fieldX + fieldWidth - (strlen(counter) * 6) - 2;
    uint8_t counterY = fieldY + fieldHeight + 2;
    
    display.drawText(counterX, counterY, counter, 1);
    
    if (textLength >= maxLength - 10) {
        display.drawRect(counterX - 1, counterY, (strlen(counter) * 6) + 2, 8, true);
        display.drawInvertedText(counterX, counterY, counter, 1);
    }
}

void InputField::drawModeIndicator(void) {
    uint8_t modeY = fieldY + fieldHeight + 2;
    display.drawText(fieldX + 2, modeY, modeIndicator, 1);
}

// ============================================
// UTILITAIRES
// ============================================
uint8_t InputField::calculateVisibleChars(void) {
    return (fieldWidth - 4) / 6;
}

void InputField::adjustScroll(void) {
    visibleChars = calculateVisibleChars();
    
    if (cursorPos < scrollOffset) {
        scrollOffset = cursorPos;
    }
    
    if (cursorPos >= scrollOffset + visibleChars) {
        scrollOffset = cursorPos - visibleChars + 1;
    }
    
    if (scrollOffset > textLength) {
        scrollOffset = (textLength > visibleChars) ? textLength - visibleChars : 0;
    }
}

void InputField::forceRedraw(void) {
    needsRedraw = true;
}

// ============================================
// FIN DU FICHIER input_field.cpp
// ============================================