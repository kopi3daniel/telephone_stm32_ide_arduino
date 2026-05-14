#include "call_manager.h"
#include "lora_driver.h"
#include "audio.h"
#include "display.h"
#include "keyboard.h"
#include <string.h>
#include <stdio.h>

extern LoRaDriver lora;
extern Audio audio;
extern Display display;
extern Keyboard keyboard;

// ============================================
// CONSTRUCTEUR
// ============================================
CallManager::CallManager() {
    currentState = CALL_STATE_IDLE;
    callLogCount = 0;
    missedCallCount = 0;
    memset(&activeCall, 0, sizeof(activeCall));
    memset(callLog, 0, sizeof(callLog));
}

// ============================================
// INITIALISATION
// ============================================
void CallManager::init(void) {
    currentState = CALL_STATE_IDLE;
    activeCall.isMuted = false;
    activeCall.isSpeakerOn = true;
}

// ============================================
// CHANGEMENT D ETAT
// ============================================
void CallManager::setState(CallState_t newState) {
    currentState = newState;
}

// ============================================
// BOUCLE PRINCIPALE
// ============================================
void CallManager::process(void) {
    // Version simplifiee pour compilation
    if (keyboard.isEndPressed()) {
        if (currentState == CALL_STATE_ACTIVE ||
            currentState == CALL_STATE_RINGING_LOCAL ||
            currentState == CALL_STATE_DIALING) {
            endCall();
        }
    }
    if (keyboard.isCallPressed()) {
        if (currentState == CALL_STATE_RINGING_LOCAL) {
            answerCall();
        }
    }
}

// ============================================
// LANCER UN APPEL SORTANT
// ============================================
bool CallManager::startCall(const char* number) {
    if (currentState != CALL_STATE_IDLE) return false;
    strncpy(activeCall.remoteNumber, number, 15);
    strncpy(activeCall.remoteName, number, 31);
    activeCall.remoteId = (uint16_t)strtol(number, NULL, 16);
    lora.setProfile(LORA_PROFILE_VOICE);
    if (!sendCallPacket(PKT_CALL_REQUEST)) {
        lora.setProfile(LORA_PROFILE_DATA);
        return false;
    }
    currentState = CALL_STATE_DIALING;
    activeCall.ringStartTime = millis();
    display.showOutgoingCall(activeCall.remoteName, activeCall.remoteNumber);
    audio.enableSpeaker(true);
    return true;
}

// ============================================
// REPONDRE A UN APPEL ENTRANT
// ============================================
bool CallManager::answerCall(void) {
    if (currentState != CALL_STATE_RINGING_LOCAL) return false;
    audio.stopTone();
    lora.setProfile(LORA_PROFILE_VOICE);
    if (!sendCallPacket(PKT_CALL_ACCEPT)) {
        lora.setProfile(LORA_PROFILE_DATA);
        return false;
    }
    currentState = CALL_STATE_CONNECTING;
    activeCall.startTime = millis();
    audio.enableMicrophone(true);
    audio.enableSpeaker(true);
    delay(100);
    currentState = CALL_STATE_ACTIVE;
    display.showCallActive(activeCall.remoteName, 0, false);
    addToCallLog(CALL_TYPE_INCOMING, true);
    return true;
}

// ============================================
// REFUSER UN APPEL ENTRANT
// ============================================
bool CallManager::rejectCall(void) {
    if (currentState != CALL_STATE_RINGING_LOCAL) return false;
    audio.stopTone();
    sendCallPacket(PKT_CALL_REJECT);
    currentState = CALL_STATE_IDLE;
    addToCallLog(CALL_TYPE_MISSED, false);
    missedCallCount++;
    display.showHomeScreen();
    lora.setProfile(LORA_PROFILE_DATA);
    return true;
}

// ============================================
// RACCROCHER
// ============================================
bool CallManager::endCall(void) {
    if (currentState != CALL_STATE_ACTIVE &&
        currentState != CALL_STATE_DIALING &&
        currentState != CALL_STATE_RINGING_LOCAL) return false;
    
    for (int i = 0; i < 3; i++) {
        sendCallPacket(PKT_CALL_END);
        delay(50);
    }
    audio.stopTone();
    audio.enableMicrophone(false);
    audio.enableSpeaker(false);
    
    uint32_t duration = 0;
    if (currentState == CALL_STATE_ACTIVE) {
        duration = (millis() - activeCall.startTime) / 1000;
    }
    currentState = CALL_STATE_ENDED;
    display.showCallEnded(activeCall.remoteName, duration);
    if (currentState == CALL_STATE_ACTIVE) {
        addToCallLog(CALL_TYPE_OUTGOING, true);
    }
    lora.setProfile(LORA_PROFILE_DATA);
    return true;
}

// ============================================
// BASCULER LE MODE MUET
// ============================================
bool CallManager::toggleMute(void) {
    activeCall.isMuted = !activeCall.isMuted;
    audio.muteMicrophone(activeCall.isMuted);
    return activeCall.isMuted;
}

// ============================================
// BASCULER LE HAUT PARLEUR
// ============================================
bool CallManager::toggleSpeaker(void) {
    activeCall.isSpeakerOn = !activeCall.isSpeakerOn;
    audio.setVolume(activeCall.isSpeakerOn ? 100 : 50);
    return activeCall.isSpeakerOn;
}

// ============================================
// ENVOYER UN PAQUET DE SIGNALISATION
// ============================================
bool CallManager::sendCallPacket(uint8_t packetType) {
    LoRaPacket_t packet;
    packet.syncByte = PKT_SYNC_BYTE;
    packet.packetType = packetType;
    packet.senderId = PHONE_ID;
    packet.targetId = activeCall.remoteId;
    packet.sequenceNum = 0;
    packet.payloadSize = 0;
    
    if (packetType == PKT_CALL_REQUEST) {
        strncpy((char*)packet.payload, PHONE_NUMBER, 16);
        packet.payloadSize = strlen(PHONE_NUMBER);
    }
    
    return lora.sendPacket(&packet);
}

// ============================================
// TRAITEMENT DES PAQUETS RECUS
// ============================================
void CallManager::handleCallPacket(uint8_t packetType, uint16_t senderId, 
                                   uint8_t* payload, uint8_t payloadSize) {
    if (packetType == PKT_CALL_REQUEST) {
        if (currentState != CALL_STATE_IDLE) {
            activeCall.remoteId = senderId;
            sendCallPacket(PKT_CALL_BUSY);
            return;
        }
        activeCall.remoteId = senderId;
        if (payloadSize > 0 && payloadSize < 16) {
            memcpy(activeCall.remoteNumber, payload, payloadSize);
            activeCall.remoteNumber[payloadSize] = '\0';
        }
        strncpy(activeCall.remoteName, "Inconnu", 31);
        currentState = CALL_STATE_RINGING_LOCAL;
        activeCall.ringStartTime = millis();
        display.showIncomingCall(activeCall.remoteName, activeCall.remoteNumber);
        audio.enableSpeaker(true);
        audio.playRingTone();
        sendCallPacket(PKT_CALL_RINGING);
    }
    // Autres types ignores pour l instant
}

// ============================================
// TRAITEMENT DE LA VOIX
// ============================================
void CallManager::processVoiceTX(void) {
    // A implementer
}

void CallManager::processVoiceRX(void) {
    // A implementer
}

void CallManager::handleVoicePacket(uint8_t* data, uint8_t length) {
    // A implementer
}

// ============================================
// JOURNAL D APPELS
// ============================================
void CallManager::addToCallLog(CallType_t type, bool answered) {
    if (callLogCount >= 50) {
        for (int i = 0; i < 49; i++) callLog[i] = callLog[i + 1];
        callLogCount = 49;
    }
    strncpy(callLog[callLogCount].number, activeCall.remoteNumber, 15);
    strncpy(callLog[callLogCount].name, activeCall.remoteName, 31);
    callLog[callLogCount].type = type;
    callLog[callLogCount].timestamp = millis();
    callLog[callLogCount].answered = answered;
    callLog[callLogCount].duration = 0;
    callLogCount++;
    if (type == CALL_TYPE_MISSED) missedCallCount++;
}

// ============================================
// INFORMATIONS
// ============================================
CallState_t CallManager::getState(void) { return currentState; }
const char* CallManager::getRemoteNumber(void) { return activeCall.remoteNumber; }
const char* CallManager::getRemoteName(void) { return activeCall.remoteName; }
uint32_t CallManager::getCallDuration(void) {
    if (currentState == CALL_STATE_ACTIVE) return (millis() - activeCall.startTime) / 1000;
    return 0;
}
bool CallManager::isMuted(void) { return activeCall.isMuted; }
bool CallManager::isSpeakerOn(void) { return activeCall.isSpeakerOn; }
uint8_t CallManager::getCallLogCount(void) { return callLogCount; }
CallLogEntry_t* CallManager::getCallLogEntry(uint8_t index) {
    if (index < callLogCount) return &callLog[index];
    return NULL;
}
uint8_t CallManager::getMissedCallCount(void) { return missedCallCount; }
void CallManager::clearMissedCallCount(void) { missedCallCount = 0; }
void CallManager::clearCallLog(void) { memset(callLog, 0, sizeof(callLog)); callLogCount = 0; missedCallCount = 0; }
const char* CallManager::findContactName(const char* number) { return NULL; }
void CallManager::formatDuration(uint32_t seconds, char* buffer, uint8_t bufferSize) {
    snprintf(buffer, bufferSize, "%02lu:%02lu", seconds / 60, seconds % 60);
}