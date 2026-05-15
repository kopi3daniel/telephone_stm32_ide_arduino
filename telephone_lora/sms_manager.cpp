

#include "sms_manager.h"
#include "lora_driver.h"
#include "display.h"
#include "audio.h"
#include <string.h>
#include <stdio.h>

// References externes (definies dans le .ino principal)
extern LoRaDriver lora;
extern Display display;
extern Audio audio;

// ============================================
// CONSTRUCTEUR
// ============================================
SMSManager::SMSManager() {
    currentState = SMS_STATE_IDLE;
    conversationCount = 0;
    selectedConversation = 0;
    nextMessageId = 1;
    totalUnreadMessages = 0;
    
    memset(composingText, 0, sizeof(composingText));
    memset(composingRecipient, 0, sizeof(composingRecipient));
    composingIndex = 0;
    
    memset(conversations, 0, sizeof(conversations));
    memset(&fragmentAssembly, 0, sizeof(fragmentAssembly));
}

// ============================================
// INITIALISATION
// ============================================
void SMSManager::init(void) {
    load();
    currentState = SMS_STATE_IDLE;
    clearMessage();
}

// ============================================
// COMPOSITION D UN NOUVEAU MESSAGE
// ============================================

void SMSManager::newMessage(void) {
    clearMessage();
    currentState = SMS_STATE_COMPOSING;
}

void SMSManager::setRecipient(const char* recipient) {
    if (recipient != NULL) {
        strncpy(composingRecipient, recipient, 15);
        composingRecipient[15] = '\0';
    }
}

void SMSManager::addChar(char c) {
    if (composingIndex < SMS_MAX_LENGTH) {
        composingText[composingIndex] = c;
        composingIndex++;
        composingText[composingIndex] = '\0';
    }
}

void SMSManager::deleteChar(void) {
    if (composingIndex > 0) {
        composingIndex--;
        composingText[composingIndex] = '\0';
    }
}

void SMSManager::clearMessage(void) {
    memset(composingText, 0, sizeof(composingText));
    memset(composingRecipient, 0, sizeof(composingRecipient));
    composingIndex = 0;
    currentState = SMS_STATE_IDLE;
}

const char* SMSManager::getComposingText(void) {
    return composingText;
}

uint16_t SMSManager::getComposingLength(void) {
    return composingIndex;
}

uint16_t SMSManager::getRemainingChars(void) {
    return SMS_MAX_LENGTH - composingIndex;
}

// ============================================
// ENVOI DU MESSAGE
// ============================================
bool SMSManager::sendMessage(void) {
    
    if (composingIndex == 0 || strlen(composingRecipient) == 0) {
        return false;
    }
    
    currentState = SMS_STATE_SENDING;
    
    uint16_t targetId = (uint16_t)strtol(composingRecipient, NULL, 16);
    uint16_t msgId = generateMessageId();
    uint16_t msgLength = composingIndex;
    
    if (msgLength <= SMS_MAX_LENGTH) {
        // === MESSAGE COURT ===
        
        if (sendSMSPacket(targetId, msgId, 1, 1, composingText, msgLength)) {
            Message_t msg;
            msg.messageId = msgId;
            strncpy(msg.sender, PHONE_NUMBER, 15);
            strncpy(msg.recipient, composingRecipient, 15);
            strncpy(msg.text, composingText, SMS_MAX_LENGTH);
            msg.text[SMS_MAX_LENGTH] = '\0';
            msg.timestamp = millis();
            msg.read = true;
            msg.outgoing = true;
            msg.delivered = false;
            
            addMessageToConversation(composingRecipient, &msg);
            
            display.showMessageSent(true);
            audio.playNotificationBeep();
            
            clearMessage();
            currentState = SMS_STATE_SENT;
            save();
            return true;
        }
        
    } else {
        // === MESSAGE LONG : fragmentation ===
        
        uint8_t totalFragments = (msgLength + 159) / 160;
        
        bool allSent = true;
        for (uint8_t i = 0; i < totalFragments; i++) {
            uint16_t start = i * 160;
            uint16_t len = (i == totalFragments - 1) ? msgLength - start : 160;
            
            if (!sendSMSPacket(targetId, msgId, i + 1, totalFragments, 
                              composingText + start, len)) {
                allSent = false;
                break;
            }
            delay(50);
        }
        
        if (allSent) {
            Message_t msg;
            msg.messageId = msgId;
            strncpy(msg.sender, PHONE_NUMBER, 15);
            strncpy(msg.recipient, composingRecipient, 15);
            strncpy(msg.text, composingText, SMS_MAX_LENGTH);
            msg.timestamp = millis();
            msg.read = true;
            msg.outgoing = true;
            msg.delivered = false;
            
            addMessageToConversation(composingRecipient, &msg);
            
            display.showMessageSent(true);
            audio.playNotificationBeep();
            clearMessage();
            currentState = SMS_STATE_SENT;
            save();
            return true;
        }
    }
    
    currentState = SMS_STATE_FAILED;
    display.showMessageSent(false);
    return false;
}

bool SMSManager::retrySend(void) {
    if (currentState == SMS_STATE_FAILED) {
        composingIndex = strlen(composingText);
        return sendMessage();
    }
    return false;
}

SMSState_t SMSManager::getState(void) {
    return currentState;
}

// ============================================
// ENVOI DU PAQUET SMS VIA LORA
// ============================================
bool SMSManager::sendSMSPacket(uint16_t targetId, uint16_t messageId,
                              uint8_t fragmentNum, uint8_t totalFragments,
                              const char* text, uint8_t length) {
    
    lora.setProfile(LORA_PROFILE_DATA);
    
    LoRaPacket_t packet;
    packet.syncByte = PKT_SYNC_BYTE;
    packet.packetType = PKT_SMS_TEXT;
    packet.senderId = PHONE_ID;
    packet.targetId = targetId;
    packet.sequenceNum = messageId;
    
    uint8_t offset = 0;
    packet.payload[offset++] = (messageId >> 8) & 0xFF;
    packet.payload[offset++] = messageId & 0xFF;
    packet.payload[offset++] = fragmentNum;
    packet.payload[offset++] = totalFragments;
    
    if (length > 160) length = 160;
    memcpy(packet.payload + offset, text, length);
    offset += length;
    
    packet.payloadSize = offset;
    
    bool result = lora.sendPacket(&packet);
    lora.startReceive();
    
    return result;
}

// ============================================
// ENVOI D UN ACCUSE DE RECEPTION
// ============================================
bool SMSManager::sendAcknowledgment(uint16_t senderId, uint16_t messageId) {
    
    LoRaPacket_t packet;
    packet.syncByte = PKT_SYNC_BYTE;
    packet.packetType = PKT_SMS_ACK;
    packet.senderId = PHONE_ID;
    packet.targetId = senderId;
    packet.sequenceNum = messageId;
    
    packet.payload[0] = (messageId >> 8) & 0xFF;
    packet.payload[1] = messageId & 0xFF;
    packet.payloadSize = 2;
    
    return lora.sendPacket(&packet);
}

// ============================================
// RECEPTION D UN SMS
// ============================================
void SMSManager::handleSMSPacket(uint16_t senderId, uint8_t* payload, uint8_t payloadSize) {
    
    if (payloadSize < 5) return;
    
    uint16_t messageId = (payload[0] << 8) | payload[1];
    uint8_t fragmentNum = payload[2];
    uint8_t totalFragments = payload[3];
    
    uint8_t textLength = payloadSize - 4;
    char* text = (char*)(payload + 4);
    
    sendAcknowledgment(senderId, messageId);
    
    char senderNumber[16];
    sprintf(senderNumber, "%04X", senderId);
    
    if (totalFragments == 1) {
        // === MESSAGE SIMPLE ===
        
        Message_t msg;
        msg.messageId = messageId;
        strncpy(msg.sender, senderNumber, 15);
        strncpy(msg.recipient, PHONE_NUMBER, 15);
        
        uint8_t copyLen = (textLength > SMS_MAX_LENGTH) ? SMS_MAX_LENGTH : textLength;
        memcpy(msg.text, text, copyLen);
        msg.text[copyLen] = '\0';
        
        msg.timestamp = millis();
        msg.read = false;
        msg.outgoing = false;
        msg.delivered = true;
        
        addMessageToConversation(senderNumber, &msg);
        
        totalUnreadMessages++;
        display.showNewMessageIcon(true);
        audio.playNotificationBeep();
        
        char preview[40];
        snprintf(preview, sizeof(preview), "Nouveau message de %s", senderNumber);
        display.showAlert("SMS recu", preview);
        
    } else {
        // === MESSAGE FRAGMENTE ===
        
        if (assembleFragment(messageId, fragmentNum, totalFragments, text, textLength)) {
            
            Message_t msg;
            msg.messageId = messageId;
            strncpy(msg.sender, senderNumber, 15);
            strncpy(msg.recipient, PHONE_NUMBER, 15);
            
            uint16_t copyLen = (fragmentAssembly.totalLength > SMS_MAX_LENGTH) ? 
                              SMS_MAX_LENGTH : fragmentAssembly.totalLength;
            memcpy(msg.text, fragmentAssembly.assemblyBuffer, copyLen);
            msg.text[copyLen] = '\0';
            
            msg.timestamp = millis();
            msg.read = false;
            msg.outgoing = false;
            msg.delivered = true;
            
            addMessageToConversation(senderNumber, &msg);
            
            totalUnreadMessages++;
            display.showNewMessageIcon(true);
            audio.playNotificationBeep();
            
            char preview[40];
            snprintf(preview, sizeof(preview), "Long message de %s", senderNumber);
            display.showAlert("SMS recu", preview);
            
            clearFragmentAssembly();
        }
    }
    
    save();
}

// ============================================
// ASSEMBLAGE D UN MESSAGE LONG
// ============================================
bool SMSManager::assembleFragment(uint16_t messageId, uint8_t fragmentNum,
                                 uint8_t totalFragments, const char* data, uint8_t length) {
    
    // Verifier si c est un nouvel assemblage
    if (!fragmentAssembly.active || fragmentAssembly.messageId != messageId) {
        // Nouveau message fragmente
        fragmentAssembly.messageId = messageId;
        fragmentAssembly.totalFragments = totalFragments;
        fragmentAssembly.receivedFragments = 0;
        fragmentAssembly.totalLength = totalFragments * 160;
        fragmentAssembly.startTime = millis();
        fragmentAssembly.active = true;
        
        memset(fragmentAssembly.assemblyBuffer, 0, sizeof(fragmentAssembly.assemblyBuffer));
    }
    
    if (fragmentNum == 0 || fragmentNum > totalFragments) return false;
    
    // Copier les donnees au bon endroit
    uint16_t offset = (fragmentNum - 1) * 160;
    if (offset + length < sizeof(fragmentAssembly.assemblyBuffer)) {
        memcpy(fragmentAssembly.assemblyBuffer + offset, data, length);
    }
    
    fragmentAssembly.receivedFragments++;
    
    // Verifier si l assemblage est complet
    if (fragmentAssembly.receivedFragments >= totalFragments) {
        fragmentAssembly.assemblyBuffer[sizeof(fragmentAssembly.assemblyBuffer) - 1] = '\0';
        fragmentAssembly.totalLength = strlen(fragmentAssembly.assemblyBuffer);
        return true;
    }
    
    // Verifier timeout (30 secondes)
    if (millis() - fragmentAssembly.startTime > 30000) {
        clearFragmentAssembly();
    }
    
    return false;
}

void SMSManager::clearFragmentAssembly(void) {
    memset(fragmentAssembly.assemblyBuffer, 0, sizeof(fragmentAssembly.assemblyBuffer));
    fragmentAssembly.active = false;
    fragmentAssembly.messageId = 0;
    fragmentAssembly.receivedFragments = 0;
}

// ============================================
// TRAITEMENT D UN ACCUSE DE RECEPTION
// ============================================
void SMSManager::handleAcknowledgment(uint16_t senderId, uint16_t messageId) {
    for (uint8_t c = 0; c < conversationCount; c++) {
        for (uint8_t m = 0; m < conversations[c].messageCount; m++) {
            if (conversations[c].messages[m].messageId == messageId &&
                conversations[c].messages[m].outgoing) {
                conversations[c].messages[m].delivered = true;
                currentState = SMS_STATE_DELIVERED;
                return;
            }
        }
    }
}

// ============================================
// GESTION DES CONVERSATIONS
// ============================================

Conversation_t* SMSManager::findConversation(const char* contact) {
    for (uint8_t i = 0; i < conversationCount; i++) {
        if (strcmp(conversations[i].contact, contact) == 0) {
            return &conversations[i];
        }
    }
    return NULL;
}

Conversation_t* SMSManager::findOrCreateConversation(const char* contact) {
    Conversation_t* conv = findConversation(contact);
    
    if (conv == NULL && conversationCount < SMS_MAX_CONVERSATIONS) {
        conv = &conversations[conversationCount];
        strncpy(conv->contact, contact, 15);
        conv->contact[15] = '\0';
        strncpy(conv->contactName, contact, 31);
        conv->contactName[31] = '\0';
        conv->messageCount = 0;
        conv->unreadCount = 0;
        conv->lastActivity = 0;
        conversationCount++;
    }
    
    return conv;
}

bool SMSManager::addMessageToConversation(const char* contact, Message_t* message) {
    Conversation_t* conv = findOrCreateConversation(contact);
    
    if (conv == NULL) return false;
    
    if (conv->messageCount >= SMS_MAX_PER_CONVERSATION) {
        for (uint8_t i = 0; i < SMS_MAX_PER_CONVERSATION - 1; i++) {
            conv->messages[i] = conv->messages[i + 1];
        }
        conv->messageCount = SMS_MAX_PER_CONVERSATION - 1;
    }
    
    conv->messages[conv->messageCount] = *message;
    conv->messageCount++;
    conv->lastActivity = message->timestamp;
    
    if (!message->read && !message->outgoing) {
        conv->unreadCount++;
    }
    
    return true;
}

uint16_t SMSManager::generateMessageId(void) {
    return nextMessageId++;
}

// ============================================
// ACCES AUX CONVERSATIONS
// ============================================

uint8_t SMSManager::getConversationCount(void) {
    return conversationCount;
}

Conversation_t* SMSManager::getConversation(uint8_t index) {
    if (index < conversationCount) {
        return &conversations[index];
    }
    return NULL;
}

const char* SMSManager::getConversationPreview(uint8_t index) {
    if (index >= conversationCount) return "";
    
    Conversation_t* conv = &conversations[index];
    if (conv->messageCount == 0) return "";
    
    return conv->messages[conv->messageCount - 1].text;
}

void SMSManager::selectConversation(uint8_t index) {
    if (index < conversationCount) {
        selectedConversation = index;
    }
}

Conversation_t* SMSManager::getSelectedConversation(void) {
    if (selectedConversation < conversationCount) {
        return &conversations[selectedConversation];
    }
    return NULL;
}

void SMSManager::deleteConversation(uint8_t index) {
    if (index >= conversationCount) return;
    
    totalUnreadMessages -= conversations[index].unreadCount;
    
    for (uint8_t i = index; i < conversationCount - 1; i++) {
        conversations[i] = conversations[i + 1];
    }
    
    conversationCount--;
    memset(&conversations[conversationCount], 0, sizeof(Conversation_t));
}

// ============================================
// NOTIFICATIONS
// ============================================

uint8_t SMSManager::getUnreadCount(void) {
    return totalUnreadMessages;
}

void SMSManager::markAsRead(uint8_t index) {
    if (index >= conversationCount) return;
    
    Conversation_t* conv = &conversations[index];
    totalUnreadMessages -= conv->unreadCount;
    conv->unreadCount = 0;
    
    for (uint8_t i = 0; i < conv->messageCount; i++) {
        conv->messages[i].read = true;
    }
    
    if (totalUnreadMessages == 0) {
        display.showNewMessageIcon(false);
    }
}

void SMSManager::markAllAsRead(void) {
    for (uint8_t i = 0; i < conversationCount; i++) {
        markAsRead(i);
    }
}

bool SMSManager::hasNewMessages(void) {
    return (totalUnreadMessages > 0);
}

// ============================================
// SAUVEGARDE / CHARGEMENT
// ============================================

void SMSManager::save(void) {
    // TODO: implementer la sauvegarde en EEPROM/Flash
}

void SMSManager::load(void) {
    // TODO: implementer le chargement depuis EEPROM/Flash
}

void SMSManager::clearAll(void) {
    memset(conversations, 0, sizeof(conversations));
    conversationCount = 0;
    totalUnreadMessages = 0;
    selectedConversation = 0;
    save();
}

// ============================================
// FIN DU FICHIER sms_manager.cpp
// ============================================