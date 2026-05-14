/**
 * ---------------------------------------------------------------------------
 * sms_screen.cpp - Implementation de l ecran SMS
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "sms_screen.h"
#include "display.h"
#include "keyboard.h"
#include <string.h>
#include <stdio.h>

extern Display display;
extern Keyboard keyboard;

// ============================================
// CONSTRUCTEUR
// ============================================
SMSScreen::SMSScreen() {
    mode = SMS_MODE_LIST;
    isActive = false;
    lastUpdate = 0;
    
    conversationCount = 0;
    selectedConversation = 0;
    conversationScroll = 0;
    messageScroll = 0;
    selectedMessage = 0;
    
    memset(conversations, 0, sizeof(conversations));
    memset(editText, 0, sizeof(editText));
    memset(editRecipient, 0, sizeof(editRecipient));
    editIndex = 0;
    charsRemaining = SMS_MAX_LENGTH;
}

void SMSScreen::init(void) {
    isActive = false;
    mode = SMS_MODE_LIST;
}

// ============================================
// CYCLE DE VIE
// ============================================
void SMSScreen::onEnter(void) {
    isActive = true;
    loadConversations();
    draw();
}

void SMSScreen::onExit(void) {
    isActive = false;
}

void SMSScreen::update(void) {
    if (!isActive) return;
}

// ============================================
// DESSIN PRINCIPAL
// ============================================
void SMSScreen::draw(void) {
    display.clear();
    
    switch (mode) {
        case SMS_MODE_LIST:
            drawListMode();
            break;
        case SMS_MODE_CONVERSATION:
            drawConversationMode();
            break;
        case SMS_MODE_EDITOR:
            drawEditorMode();
            break;
        case SMS_MODE_SENDING:
            drawSendingMode();
            break;
        case SMS_MODE_SENT:
            drawSentMode();
            break;
        case SMS_MODE_FAILED:
            drawFailedMode();
            break;
    }
    
    display.refresh();
}

// ============================================
// MODE LISTE DES CONVERSATIONS
// ============================================
void SMSScreen::drawListMode(void) {
    drawHeader("Messages");
    
    if (conversationCount == 0) {
        display.drawCenteredText(30, "Aucun message", 1);
        display.drawCenteredText(40, "Appuyez sur D", 1);
        display.drawCenteredText(48, "pour ecrire", 1);
    } else {
        uint8_t visibleCount = (conversationCount < SMS_MESSAGES_PER_PAGE) ? 
                               conversationCount : SMS_MESSAGES_PER_PAGE;
        
        for (uint8_t i = 0; i < visibleCount; i++) {
            uint8_t convIdx = conversationScroll + i;
            if (convIdx >= conversationCount) break;
            
            uint8_t y = 12 + (i * 11);
            bool isSelected = (convIdx == selectedConversation);
            drawConversationPreview(convIdx, y, isSelected);
        }
    }
    
    drawInstructions("VERT:Ouvrir", "C:Suppr", "D:Nouveau");
}

void SMSScreen::drawConversationPreview(uint8_t index, uint8_t y, bool selected) {
    if (index >= conversationCount) return;
    
    SMSConversation_t* conv = &conversations[index];
    
    if (selected) {
        display.drawRect(0, y - 1, SCREEN_WIDTH, 11, true);
    }
    
    // Nom du contact
    char displayName[34];
    if (conv->unreadCount > 0) {
        snprintf(displayName, sizeof(displayName), "* %s", conv->contactName);
    } else {
        snprintf(displayName, sizeof(displayName), "  %s", conv->contactName);
    }
    
    if (selected) {
        display.drawInvertedText(2, y, displayName, 1);
    } else {
        display.drawText(2, y, displayName, 1);
    }
    
    // Apercu du dernier message
    if (conv->messageCount > 0) {
        SMSMessage_t* lastMsg = &conv->messages[conv->messageCount - 1];
        char preview[26];
        strncpy(preview, lastMsg->text, 22);
        preview[22] = '\0';
        if (strlen(lastMsg->text) > 22) {
            strcat(preview, "...");
        }
        
        if (selected) {
            display.drawInvertedText(2, y + 1, preview, 1);
        } else {
            display.drawText(2, y + 1, preview, 1);
        }
    }
    
    // Badge messages non lus
    if (conv->unreadCount > 0) {
        char badge[5];
        snprintf(badge, sizeof(badge), "%d", conv->unreadCount);
        uint8_t badgeX = SCREEN_WIDTH - (strlen(badge) * 6) - 4;
        
        if (selected) {
            display.drawInvertedText(badgeX, y, badge, 1);
        } else {
            display.drawRect(badgeX - 1, y, (strlen(badge) * 6) + 2, 9, true);
            display.drawInvertedText(badgeX, y, badge, 1);
        }
    }
}

// ============================================
// MODE CONVERSATION (BULLES)
// ============================================
void SMSScreen::drawConversationMode(void) {
    if (selectedConversation >= conversationCount) return;
    
    SMSConversation_t* conv = &conversations[selectedConversation];
    
    drawHeader(conv->contactName);
    
    uint8_t y = 12;
    uint8_t startMsg = (conv->messageCount > SMS_MESSAGES_PER_PAGE) ? 
                       conv->messageCount - SMS_MESSAGES_PER_PAGE : 0;
    
    for (uint8_t i = startMsg; i < conv->messageCount; i++) {
        drawMessageBubble(i, y, &conv->messages[i]);
        
        uint8_t lines = (strlen(conv->messages[i].text) / 20) + 1;
        y += (lines * 9) + 2;
        
        if (y > SCREEN_HEIGHT - 12) break;
    }
    
    drawInstructions("VERT:Repondre", "", "ROUGE:Retour");
}

void SMSScreen::drawMessageBubble(uint8_t index, uint8_t y, SMSMessage_t* msg) {
    uint8_t bubbleX, bubbleW;
    uint8_t textX;
    
    if (msg->outgoing) {
        bubbleX = SCREEN_WIDTH - 100;
        bubbleW = 96;
        textX = bubbleX + 3;
    } else {
        bubbleX = 2;
        bubbleW = 96;
        textX = bubbleX + 3;
    }
    
    display.drawRect(bubbleX, y, bubbleW, 10, false);
    
    char line[22];
    strncpy(line, msg->text, 21);
    line[21] = '\0';
    display.drawText(textX, y + 1, line, 1);
    
    if (msg->timestamp > 0) {
        char timeStr[6];
        formatTime(timeStr, msg->timestamp);
        display.drawText(textX, y + 10, timeStr, 1);
    }
    
    if (msg->outgoing && msg->read) {
        display.drawText(bubbleX + bubbleW - 8, y + 10, "v", 1);
    }
}

// ============================================
// MODE EDITEUR
// ============================================
void SMSScreen::drawEditorMode(void) {
    drawHeader("Nouveau message");
    
    display.drawText(2, 12, "A:", 1);
    display.drawText(20, 12, editRecipient, 1);
    display.drawLine(0, 21, SCREEN_WIDTH, 21);
    
    display.drawText(2, 24, editText, 1);
    
    uint8_t cursorX = 2 + (editIndex * 6);
    if (cursorX < SCREEN_WIDTH - 2) {
        display.drawLine(cursorX, 24, cursorX, 32);
    }
    
    drawCharCounter();
    
    const char* modeStr = keyboard.getModeString();
    display.drawText(2, 42, modeStr, 1);
    
    drawInstructions("VERT:Envoyer", "C:Effacer", "ROUGE:Annuler");
}

void SMSScreen::drawCharCounter(void) {
    char counter[10];
    snprintf(counter, sizeof(counter), "%d/%d", editIndex, SMS_MAX_LENGTH);
    
    uint8_t x = SCREEN_WIDTH - (strlen(counter) * 6) - 2;
    display.drawText(x, 36, counter, 1);
    
    uint8_t barW = (editIndex * 40) / SMS_MAX_LENGTH;
    display.drawRect(SCREEN_WIDTH - 42, 44, 40, 4, false);
    display.drawRect(SCREEN_WIDTH - 42, 44, barW, 4, true);
}

// ============================================
// MODE ENVOI
// ============================================
void SMSScreen::drawSendingMode(void) {
    drawHeader("Envoi...");
    display.drawCenteredText(30, "Envoi du message...", 1);
    
    static uint8_t dots = 0;
    dots++;
    for (uint8_t i = 0; i < (dots % 4); i++) {
        display.drawRect(60 + (i * 8), 40, 4, 4, true);
    }
}

void SMSScreen::drawSentMode(void) {
    drawHeader("Envoye !");
    display.drawCenteredText(30, "Message envoye !", 1);
    delay(1500);
    mode = SMS_MODE_CONVERSATION;
    draw();
}

void SMSScreen::drawFailedMode(void) {
    drawHeader("Erreur");
    display.drawCenteredText(30, "Echec d envoi", 1);
    display.drawCenteredText(40, "Reessayer ?", 1);
    drawInstructions("VERT:Oui", "", "ROUGE:Non");
}

// ============================================
// DESSIN UTILITAIRES
// ============================================
void SMSScreen::drawHeader(const char* title) {
    display.drawCenteredText(0, title, 1);
    display.drawLine(0, 9, SCREEN_WIDTH, 9);
}

void SMSScreen::drawInstructions(const char* left, const char* center, const char* right) {
    if (left) display.drawText(0, SCREEN_HEIGHT - 10, left, 1);
    if (center) display.drawCenteredText(SCREEN_HEIGHT - 10, center, 1);
    if (right) {
        uint8_t x = SCREEN_WIDTH - (strlen(right) * 6) - 2;
        display.drawText(x, SCREEN_HEIGHT - 10, right, 1);
    }
}

void SMSScreen::formatTime(char* buffer, uint32_t timestamp) {
    uint32_t totalMinutes = timestamp / 60000;
    uint8_t hours = (totalMinutes / 60) % 24;
    uint8_t minutes = totalMinutes % 60;
    snprintf(buffer, 6, "%02d:%02d", hours, minutes);
}

// ============================================
// GESTION DES TOUCHES
// ============================================
uint8_t SMSScreen::handleKeyPress(char key) {
    if (!isActive) return 0;
    
    switch (mode) {
        
        case SMS_MODE_LIST:
            switch (key) {
                case '2':
                    if (selectedConversation > 0) selectedConversation--;
                    if (selectedConversation < conversationScroll) conversationScroll--;
                    draw(); return 0;
                case '8':
                    if (selectedConversation < conversationCount - 1) selectedConversation++;
                    if (selectedConversation >= conversationScroll + SMS_MESSAGES_PER_PAGE) conversationScroll++;
                    draw(); return 0;
                case 'A':
                    openConversation(selectedConversation); return 0;
                case 'C':
                    deleteConversation(selectedConversation); draw(); return 0;
                case 'D':
                    newMessage(NULL); return 0;
                case 'B':
                    return 255;
                default: return 0;
            }
        
        case SMS_MODE_CONVERSATION:
            switch (key) {
                case 'A':
                    newMessage(conversations[selectedConversation].contactNumber);
                    return 0;
                case 'B':
                    mode = SMS_MODE_LIST; draw(); return 0;
                default: return 0;
            }
        
        case SMS_MODE_EDITOR:
            switch (key) {
                case 'A':
                    sendMessage(); return 0;
                case 'C':
                    if (editIndex > 0) { editIndex--; editText[editIndex] = '\0'; }
                    draw(); return 0;
                case 'B':
                    mode = SMS_MODE_LIST; draw(); return 0;
                case 'J':
                    keyboard.toggleMode(); draw(); return 0;
                default:
                    if (key >= '0' && key <= '9') {
                        char c = keyboard.multiTapInput(key);
                        if (c != 0 && editIndex < SMS_MAX_LENGTH) {
                            editText[editIndex++] = c;
                            editText[editIndex] = '\0';
                            draw();
                        }
                    }
                    return 0;
            }
        
        default:
            return 0;
    }
}

// ============================================
// ACTIONS
// ============================================
void SMSScreen::loadConversations(void) {
    // TODO: Charger depuis le module sms_manager
    conversationCount = 0;
}

void SMSScreen::openConversation(uint8_t index) {
    if (index < conversationCount) {
        selectedConversation = index;
        markAsRead(index);
        mode = SMS_MODE_CONVERSATION;
        draw();
    }
}

void SMSScreen::newMessage(const char* recipient) {
    mode = SMS_MODE_EDITOR;
    memset(editText, 0, sizeof(editText));
    editIndex = 0;
    
    if (recipient) {
        strncpy(editRecipient, recipient, 15);
    } else {
        memset(editRecipient, 0, sizeof(editRecipient));
    }
    
    draw();
}

bool SMSScreen::sendMessage(void) {
    if (editIndex == 0) return false;
    
    mode = SMS_MODE_SENDING;
    draw();
    delay(1000);
    
    // TODO: Envoyer via sms_manager
    bool success = true;
    
    if (success) {
        mode = SMS_MODE_SENT;
        draw();
    } else {
        mode = SMS_MODE_FAILED;
        draw();
    }
    
    return success;
}

void SMSScreen::markAsRead(uint8_t index) {
    if (index < conversationCount) {
        conversations[index].unreadCount = 0;
        for (uint8_t i = 0; i < conversations[index].messageCount; i++) {
            conversations[index].messages[i].read = true;
        }
    }
}

void SMSScreen::deleteConversation(uint8_t index) {
    if (index >= conversationCount) return;
    for (uint8_t i = index; i < conversationCount - 1; i++) {
        conversations[i] = conversations[i + 1];
    }
    conversationCount--;
    if (selectedConversation >= conversationCount && conversationCount > 0) {
        selectedConversation = conversationCount - 1;
    }
}

uint8_t SMSScreen::getConversationCount(void) { return conversationCount; }
uint8_t SMSScreen::getUnreadCount(void) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < conversationCount; i++) {
        count += conversations[i].unreadCount;
    }
    return count;
}
const char* SMSScreen::getEditText(void) { return editText; }
bool SMSScreen::isActiveScreen(void) { return isActive; }

// ============================================
// FIN DU FICHIER sms_screen.cpp
// ============================================