

#include "dialog.h"
#include "display.h"
#include "keyboard.h"
#include <string.h>
#include <stdio.h>

extern Display display;
extern Keyboard keyboard;

// ============================================
// CONSTRUCTEUR
// ============================================
Dialog::Dialog() {
    type = DIALOG_TYPE_ALERT;
    result = DIALOG_RESULT_NONE;
    isVisible = false;
    needsRedraw = false;
    
    memset(title, 0, sizeof(title));
    memset(message, 0, sizeof(message));
    memset(inputText, 0, sizeof(inputText));
    inputIndex = 0;
    inputMaxLength = 20;
    
    progressPercent = 0;
    memset(progressLabel, 0, sizeof(progressLabel));
    
    showStartTime = 0;
    timeoutMs = 0;
    hasTimeout = false;
    
    showOkButton = true;
    showCancelButton = false;
    okSelected = true;
    
    animate = false;
    animFrame = 0;
    lastAnimUpdate = 0;
}

// ============================================
// MISE A JOUR
// ============================================
void Dialog::update(void) {
    if (!isVisible) return;
    
    uint32_t now = millis();
    
    // Verifier le timeout
    if (hasTimeout && (now - showStartTime) > timeoutMs) {
        result = DIALOG_RESULT_TIMEOUT;
        close();
        return;
    }
    
    // Animation
    if (animate && (now - lastAnimUpdate) > 200) {
        animFrame++;
        lastAnimUpdate = now;
        needsRedraw = true;
    }
    
    // Redessiner si necessaire
    if (needsRedraw) {
        draw();
        needsRedraw = false;
    }
}

// ============================================
// DESSIN
// ============================================
void Dialog::draw(void) {
    if (!isVisible) return;
    
    drawBackground();
    drawTitle();
    drawMessage();
    
    switch (type) {
        case DIALOG_TYPE_PROGRESS:
            drawProgressBar();
            break;
        case DIALOG_TYPE_INPUT:
            drawInputField();
            break;
        default:
            break;
    }
    
    drawButtons();
}

void Dialog::drawBackground(void) {
    // Fond semi-transparent simule (quadrillage)
    for (uint8_t y = 0; y < SCREEN_HEIGHT; y += 2) {
        for (uint8_t x = 0; x < SCREEN_WIDTH; x += 2) {
            if ((x + y) % 4 == 0) {
               // display.drawPixel(x, y);
               display.drawRect(x, y, 1, 1, true);
            }
        }
    }
    
    // Rectangle du dialogue
    display.drawRect(DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT, true);
    display.drawRect(DIALOG_X - 1, DIALOG_Y - 1, DIALOG_WIDTH + 2, DIALOG_HEIGHT + 2, false);
    
    // Fond interieur noir
    display.drawRect(DIALOG_X + 1, DIALOG_Y + 1, DIALOG_WIDTH - 2, DIALOG_HEIGHT - 2, true);
}

void Dialog::drawTitle(void) {
    if (strlen(title) > 0) {
        display.drawRect(DIALOG_X, DIALOG_Y, DIALOG_WIDTH, 10, false);
        display.drawInvertedText(DIALOG_X + DIALOG_PADDING, DIALOG_Y + 1, title, 1);
    }
}

void Dialog::drawMessage(void) {
    uint8_t msgY = DIALOG_Y + 14;
    
    char line1[32] = {0};
    char line2[32] = {0};
    
    if (strlen(message) > 25) {
        strncpy(line1, message, 25);
        line1[25] = '\0';
        strncpy(line2, message + 25, 25);
        line2[25] = '\0';
    } else {
        strncpy(line1, message, 31);
    }
    
    display.drawText(DIALOG_X + DIALOG_PADDING, msgY, line1, 1);
    if (strlen(line2) > 0) {
        display.drawText(DIALOG_X + DIALOG_PADDING, msgY + 10, line2, 1);
    }
}

void Dialog::drawButtons(void) {
    uint8_t btnY = DIALOG_Y + DIALOG_HEIGHT - 12;
    
    if (showOkButton && showCancelButton) {
        if (okSelected) {
            display.drawRect(DIALOG_X + 4, btnY, 40, 8, false);
            display.drawInvertedText(DIALOG_X + 8, btnY + 1, "OK", 1);
            display.drawText(DIALOG_X + 60, btnY + 1, "Annuler", 1);
        } else {
            display.drawText(DIALOG_X + 8, btnY + 1, "OK", 1);
            display.drawRect(DIALOG_X + 56, btnY, 48, 8, false);
            display.drawInvertedText(DIALOG_X + 60, btnY + 1, "Annuler", 1);
        }
    } else if (showOkButton) {
        display.drawRect(DIALOG_X + 35, btnY, 40, 8, true);
        display.drawInvertedText(DIALOG_X + 42, btnY + 1, "OK", 1);
    }
}

void Dialog::drawProgressBar(void) {
    uint8_t barY = DIALOG_Y + 30;
    uint8_t barX = DIALOG_X + DIALOG_PADDING;
    uint8_t barW = DIALOG_WIDTH - (DIALOG_PADDING * 2);
    
    if (strlen(progressLabel) > 0) {
        display.drawText(barX, barY - 8, progressLabel, 1);
    }
    
    display.drawRect(barX, barY, barW, 6, false);
    uint8_t filled = (progressPercent * (barW - 2)) / 100;
    if (filled > 0) {
        display.drawRect(barX + 1, barY + 1, filled, 4, true);
    }
    
    char pctStr[5];
    snprintf(pctStr, sizeof(pctStr), "%d%%", progressPercent);
    display.drawText(barX + barW + 2, barY, pctStr, 1);
}

void Dialog::drawInputField(void) {
    uint8_t inputY = DIALOG_Y + 28;
    uint8_t inputX = DIALOG_X + DIALOG_PADDING;
    uint8_t inputW = DIALOG_WIDTH - (DIALOG_PADDING * 2);
    
    display.drawRect(inputX, inputY, inputW, 10, false);
    display.drawText(inputX + 2, inputY + 1, inputText, 1);
    
    if (animFrame % 2 == 0) {
        uint8_t cursorX = inputX + 2 + (inputIndex * 6);
        display.drawLine(cursorX, inputY + 1, cursorX, inputY + 8);
    }
}

void Dialog::clearDialogArea(void) {
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, true);
}

// ============================================
// AFFICHAGE DES DIALOGUES
// ============================================
void Dialog::showAlert(const char* titleText, const char* msgText) {
    type = DIALOG_TYPE_ALERT;
    result = DIALOG_RESULT_NONE;
    isVisible = true;
    
    if (titleText) strncpy(title, titleText, 31);
    if (msgText) strncpy(message, msgText, 63);
    
    showOkButton = true;
    showCancelButton = false;
    okSelected = true;
    hasTimeout = false;
    
    showStartTime = millis();
    needsRedraw = true;
}

void Dialog::showConfirm(const char* titleText, const char* msgText) {
    type = DIALOG_TYPE_CONFIRM;
    result = DIALOG_RESULT_NONE;
    isVisible = true;
    
    if (titleText) strncpy(title, titleText, 31);
    if (msgText) strncpy(message, msgText, 63);
    
    showOkButton = true;
    showCancelButton = true;
    okSelected = true;
    hasTimeout = false;
    
    showStartTime = millis();
    needsRedraw = true;
}

void Dialog::showInput(const char* titleText, uint8_t maxLength) {
    type = DIALOG_TYPE_INPUT;
    result = DIALOG_RESULT_NONE;
    isVisible = true;
    
    if (titleText) strncpy(title, titleText, 31);
    inputMaxLength = (maxLength > 31) ? 31 : maxLength;
    clearInput();
    
    showOkButton = true;
    showCancelButton = true;
    okSelected = true;
    hasTimeout = false;
    animate = true;
    
    showStartTime = millis();
    needsRedraw = true;
}

void Dialog::showProgress(const char* label, uint8_t percent) {
    type = DIALOG_TYPE_PROGRESS;
    isVisible = true;
    
    if (label) strncpy(progressLabel, label, 31);
    progressPercent = percent;
    
    showOkButton = false;
    showCancelButton = false;
    hasTimeout = false;
    
    needsRedraw = true;
}

void Dialog::showToast(const char* msgText) {
    type = DIALOG_TYPE_TOAST;
    isVisible = true;
    
    if (msgText) strncpy(message, msgText, 63);
    title[0] = '\0';
    
    showOkButton = false;
    showCancelButton = false;
    hasTimeout = true;
    timeoutMs = 2000;
    
    showStartTime = millis();
    needsRedraw = true;
}

void Dialog::showWarning(const char* msgText) {
    type = DIALOG_TYPE_WARNING;
    isVisible = true;
    
    strncpy(title, "Attention", 31);
    if (msgText) strncpy(message, msgText, 63);
    
    showOkButton = true;
    showCancelButton = false;
    hasTimeout = false;
    
    showStartTime = millis();
    needsRedraw = true;
}

void Dialog::showError(const char* msgText) {
    type = DIALOG_TYPE_ERROR;
    isVisible = true;
    
    strncpy(title, "Erreur", 31);
    if (msgText) strncpy(message, msgText, 63);
    
    showOkButton = true;
    showCancelButton = false;
    hasTimeout = false;
    
    showStartTime = millis();
    needsRedraw = true;
}

void Dialog::showLowBattery(uint8_t percent) {
    type = DIALOG_TYPE_WARNING;
    isVisible = true;
    
    strncpy(title, "Batterie faible", 31);
    snprintf(message, sizeof(message), "Niveau: %d%% - Rechargez SVP", percent);
    
    showOkButton = true;
    showCancelButton = false;
    hasTimeout = true;
    timeoutMs = 3000;
    
    showStartTime = millis();
    needsRedraw = true;
}

// ============================================
// GESTION
// ============================================
void Dialog::close(void) {
    isVisible = false;
    animate = false;
    needsRedraw = false;
}

uint8_t Dialog::handleKeyPress(char key) {
    if (!isVisible) return 0;
    
    switch (type) {
        case DIALOG_TYPE_CONFIRM:
            switch (key) {
                case '4':
                case '6':
                    okSelected = !okSelected;
                    needsRedraw = true;
                    return 0;
                case 'A':
                    result = okSelected ? DIALOG_RESULT_OK : DIALOG_RESULT_CANCEL;
                    close();
                    return result;
                case 'B':
                    result = DIALOG_RESULT_CANCEL;
                    close();
                    return result;
                default:
                    return 0;
            }
        
        case DIALOG_TYPE_ALERT:
        case DIALOG_TYPE_WARNING:
        case DIALOG_TYPE_ERROR:
            if (key == 'A' || key == 'B') {
                result = DIALOG_RESULT_OK;
                close();
                return result;
            }
            return 0;
        
        case DIALOG_TYPE_INPUT:
            switch (key) {
                case 'A':
                    result = DIALOG_RESULT_OK;
                    close();
                    return result;
                case 'B':
                    result = DIALOG_RESULT_CANCEL;
                    clearInput();
                    close();
                    return result;
                case 'C':
                    inputDeleteChar();
                    needsRedraw = true;
                    return 0;
                default:
                    if (key >= '0' && key <= '9') {
                        char c = keyboard.multiTapInput(key);
                        if (c != 0) {
                            inputAddChar(c);
                            needsRedraw = true;
                        }
                    }
                    return 0;
            }
        
        default:
            return 0;
    }
}

bool Dialog::isDialogVisible(void) { return isVisible; }
DialogResult_t Dialog::getResult(void) { return result; }

void Dialog::updateProgress(uint8_t percent) {
    progressPercent = percent;
    needsRedraw = true;
}

// ============================================
// SAISIE DE TEXTE
// ============================================
void Dialog::inputAddChar(char c) {
    if (inputIndex < inputMaxLength) {
        inputText[inputIndex++] = c;
        inputText[inputIndex] = '\0';
    }
}

void Dialog::inputDeleteChar(void) {
    if (inputIndex > 0) {
        inputIndex--;
        inputText[inputIndex] = '\0';
    }
}

const char* Dialog::getInputText(void) { return inputText; }

void Dialog::clearInput(void) {
    memset(inputText, 0, sizeof(inputText));
    inputIndex = 0;
}

// ============================================
// FIN DU FICHIER dialog.cpp
// ============================================