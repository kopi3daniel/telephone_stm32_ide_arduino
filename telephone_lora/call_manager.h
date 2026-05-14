#ifndef CALL_MANAGER_H
#define CALL_MANAGER_H

#include <Arduino.h>
#include "config.h"

typedef enum {
    CALL_STATE_IDLE,
    CALL_STATE_DIALING,
    CALL_STATE_RINGING_LOCAL,
    CALL_STATE_RINGING_REMOTE,
    CALL_STATE_CONNECTING,
    CALL_STATE_ACTIVE,
    CALL_STATE_ENDING,
    CALL_STATE_ENDED,
    CALL_STATE_REJECTED,
    CALL_STATE_MISSED,
    CALL_STATE_BUSY
} CallState_t;

typedef enum {
    CALL_TYPE_OUTGOING,
    CALL_TYPE_INCOMING,
    CALL_TYPE_MISSED
} CallType_t;

typedef struct {
    char number[16];
    char name[32];
    CallType_t type;
    uint32_t timestamp;
    uint32_t duration;
    bool answered;
} CallLogEntry_t;

typedef struct {
    uint16_t remoteId;
    char remoteNumber[16];
    char remoteName[32];
    uint32_t startTime;
    uint32_t ringStartTime;
    bool isMuted;
    bool isSpeakerOn;
    uint32_t lastVoiceActivity;
} ActiveCall_t;

class CallManager {
    
private:
    CallState_t currentState;
    ActiveCall_t activeCall;
    
    static const uint8_t MAX_CALL_LOG = 50;
    CallLogEntry_t callLog[50];
    uint8_t callLogCount;
    uint8_t missedCallCount;
    
    static const uint32_t RING_TIMEOUT_MS = 30000;
    static const uint32_t CONNECT_TIMEOUT_MS = 5000;
    static const uint32_t END_CALL_TIMEOUT_MS = 3000;
    
    bool sendCallPacket(uint8_t packetType);
    void playStateTone(void);
    void addToCallLog(CallType_t type, bool answered);
    const char* findContactName(const char* number);
    void formatDuration(uint32_t seconds, char* buffer, uint8_t bufferSize);
    void setState(CallState_t newState);
    
public:
    CallManager();
    void init(void);
    void process(void);
    
    bool startCall(const char* number);
    bool answerCall(void);
    bool rejectCall(void);
    bool endCall(void);
    bool toggleMute(void);
    bool toggleSpeaker(void);
    
    void handleCallPacket(uint8_t packetType, uint16_t senderId, uint8_t* payload, uint8_t payloadSize);
    void handleVoicePacket(uint8_t* data, uint8_t length);
    
    void processVoiceTX(void);
    void processVoiceRX(void);
    
    CallState_t getState(void);
    const char* getRemoteNumber(void);
    const char* getRemoteName(void);
    uint32_t getCallDuration(void);
    bool isMuted(void);
    bool isSpeakerOn(void);
    
    uint8_t getCallLogCount(void);
    CallLogEntry_t* getCallLogEntry(uint8_t index);
    uint8_t getMissedCallCount(void);
    void clearMissedCallCount(void);
    void clearCallLog(void);
};

#endif