/**
 * Ce module fournit des structures de buffers pour :
 * - Buffer audio (capture micro + lecture haut-parleur)
 * - Buffer de paquets LoRa (file d attente emission/reception)
 * - Buffer circulaire generique (FIFO)
 * - Buffer de messages SMS
 * - Buffer d evenements (touches, interruptions)
 */

#ifndef BUFFERS_H
#define BUFFERS_H

#include <Arduino.h>
#include "config.h"
#include <string.h>

// ============================================
// TAILLES DES BUFFERS
// ============================================
#define AUDIO_BUFFER_SIZE       128
#define LORA_TX_QUEUE_SIZE      8
#define LORA_RX_QUEUE_SIZE      8
#define EVENT_QUEUE_SIZE        32
#define SMS_BUFFER_SIZE         256

// ============================================
// BUFFER CIRCULAIRE GENERIQUE (FIFO)
// ============================================
template<typename T, uint16_t SIZE>
class CircularBuffer {
    
private:
    T buffer[SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    bool overflow;
    
public:
    uint16_t copyTo(T* output, uint16_t maxOutput) const;
    bool find(const T& item, uint16_t* foundIndex = NULL) const;
    bool removeAt(uint16_t index);
    
    CircularBuffer() {
        head = 0;
        tail = 0;
        count = 0;
        overflow = false;
        memset(buffer, 0, sizeof(buffer));
    }
    
    bool push(const T& item) {
        if (isFull()) {
            overflow = true;
            return false;
        }
        buffer[head] = item;
        head = (head + 1) % SIZE;
        count++;
        return true;
    }
    
    void pushOverwrite(const T& item) {
        if (isFull()) {
            tail = (tail + 1) % SIZE;
            count--;
        }
        buffer[head] = item;
        head = (head + 1) % SIZE;
        count++;
    }
    
    bool pop(T& item) {
        if (isEmpty()) return false;
        item = buffer[tail];
        tail = (tail + 1) % SIZE;
        count--;
        return true;
    }
    
    bool peek(T& item) const {
        if (isEmpty()) return false;
        item = buffer[tail];
        return true;
    }
    
    bool peekAt(uint16_t index, T& item) const {
        if (index >= count) return false;
        uint16_t actualIndex = (tail + index) % SIZE;
        item = buffer[actualIndex];
        return true;
    }
    
    void clear(void) {
        head = 0;
        tail = 0;
        count = 0;
        overflow = false;
    }
    
    bool isEmpty(void) const { return (count == 0); }
    bool isFull(void) const { return (count >= SIZE); }
    uint16_t size(void) const { return count; }
    uint16_t capacity(void) const { return SIZE; }
    uint16_t available(void) const { return SIZE - count; }
    bool hasOverflowed(void) const { return overflow; }
    void clearOverflow(void) { overflow = false; }
};

// ============================================
// BUFFER AUDIO
// ============================================
class AudioBuffer {
    
private:
    uint8_t buffer[AUDIO_BUFFER_SIZE];
    uint16_t writeIndex;
    uint16_t readIndex;
    uint16_t dataCount;
    bool ready;
    
public:
    uint16_t fillFromADC(uint16_t maxSamples);
    uint16_t drainToSpeaker(uint16_t maxSamples);

    AudioBuffer() {
        writeIndex = 0;
        readIndex = 0;
        dataCount = 0;
        ready = false;
        memset(buffer, 0, sizeof(buffer));
    }
    
    uint16_t write(const uint8_t* data, uint16_t length) {
        uint16_t written = 0;
        for (uint16_t i = 0; i < length && dataCount < AUDIO_BUFFER_SIZE; i++) {
            buffer[writeIndex] = data[i];
            writeIndex = (writeIndex + 1) % AUDIO_BUFFER_SIZE;
            dataCount++;
            written++;
        }
        if (dataCount >= AUDIO_BUFFER_SIZE / 2) ready = true;
        return written;
    }
    
    uint16_t read(uint8_t* data, uint16_t maxLength) {
        uint16_t readCount = 0;
        for (uint16_t i = 0; i < maxLength && dataCount > 0; i++) {
            data[i] = buffer[readIndex];
            readIndex = (readIndex + 1) % AUDIO_BUFFER_SIZE;
            dataCount--;
            readCount++;
        }
        if (dataCount < AUDIO_BUFFER_SIZE / 4) ready = false;
        return readCount;
    }
    
    bool isReady(void) const { return ready; }
    uint16_t available(void) const { return dataCount; }
    void clear(void) {
        writeIndex = 0;
        readIndex = 0;
        dataCount = 0;
        ready = false;
    }
};

// ============================================
// BUFFER DE PAQUETS LORA
// ============================================
template<uint16_t QUEUE_SIZE>
class LoRaPacketQueue {
    
private:
    struct QueuedPacket {
        uint8_t data[PKT_MAX_PAYLOAD + PKT_HEADER_SIZE];
        uint16_t length;
        uint32_t timestamp;
        uint8_t priority;
        uint8_t retryCount;
    };
    
    QueuedPacket queue[QUEUE_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    
public:
    uint16_t cleanupExpired(uint32_t timeoutMs);
    bool incrementRetry(uint16_t index);
    LoRaPacketQueue() {
        head = 0;
        tail = 0;
        count = 0;
        memset(queue, 0, sizeof(queue));
    }
    
    bool enqueue(const uint8_t* data, uint16_t length, uint8_t priority = 0) {
        if (count >= QUEUE_SIZE) {
            if (priority > 0) {
                for (uint16_t i = 0; i < count; i++) {
                    uint16_t idx = (tail + i) % QUEUE_SIZE;
                    if (queue[idx].priority == 0) {
                        memcpy(queue[idx].data, data, length);
                        queue[idx].length = length;
                        queue[idx].timestamp = millis();
                        queue[idx].priority = priority;
                        queue[idx].retryCount = 0;
                        return true;
                    }
                }
            }
            return false;
        }
        
        memcpy(queue[head].data, data, length);
        queue[head].length = length;
        queue[head].timestamp = millis();
        queue[head].priority = priority;
        queue[head].retryCount = 0;
        
        head = (head + 1) % QUEUE_SIZE;
        count++;
        return true;
    }
    
    bool dequeue(uint8_t* data, uint16_t* length) {
        if (count == 0) return false;
        memcpy(data, queue[tail].data, queue[tail].length);
        *length = queue[tail].length;
        tail = (tail + 1) % QUEUE_SIZE;
        count--;
        return true;
    }
    
    uint16_t size(void) const { return count; }
    bool isEmpty(void) const { return (count == 0); }
    void clear(void) { head = 0; tail = 0; count = 0; }
};

// ============================================
// BUFFER D EVENEMENTS
// ============================================
typedef enum {
    EVENT_NONE,
    EVENT_KEY_PRESS,
    EVENT_KEY_RELEASE,
    EVENT_LORA_RX_DONE,
    EVENT_LORA_TX_DONE,
    EVENT_CALL_INCOMING,
    EVENT_CALL_ENDED,
    EVENT_SMS_RECEIVED,
    EVENT_BATTERY_LOW,
    EVENT_TIMER_EXPIRED,
    EVENT_POWER_BUTTON
} EventType_t;

typedef struct {
    EventType_t type;
    uint32_t timestamp;
    uint8_t data[8];
    uint8_t dataLength;
} Event_t;

class EventQueue {
    
private:
    CircularBuffer<Event_t, EVENT_QUEUE_SIZE> events;
    
public:
    bool pushKeyEvent(char key, bool pressed);
    bool pushLoRaEvent(EventType_t type, uint8_t rssi);
    uint16_t filterByType(EventType_t type, Event_t* output, uint16_t maxOutput);
    uint16_t removeByType(EventType_t type);

    EventQueue() {}
    
    bool pushEvent(EventType_t type, const uint8_t* data = NULL, uint8_t dataLength = 0) {
        Event_t event;
        event.type = type;
        event.timestamp = millis();
        event.dataLength = (dataLength > 8) ? 8 : dataLength;
        if (data != NULL && dataLength > 0) {
            memcpy(event.data, data, event.dataLength);
        }
        return events.push(event);
    }
    
    bool popEvent(Event_t& event) { return events.pop(event); }
    bool hasEvents(void) const { return !events.isEmpty(); }
    uint16_t pendingCount(void) const { return events.size(); }
    void clear(void) { events.clear(); }
};

// ============================================
// BUFFER DE MESSAGES SMS
// ============================================
class SMSBuffer {
    
private:
    char text[SMS_BUFFER_SIZE];
    uint16_t length;
    uint16_t cursorPos;
    
public:
    void truncate(void);
    void insertNewLine(void);
    uint16_t wordCount(void) const;
    bool getWord(uint16_t wordIndex, char* buffer, uint16_t bufferSize) const;
    SMSBuffer() {
        memset(text, 0, sizeof(text));
        length = 0;
        cursorPos = 0;
    }
    
    void insertChar(char c) {
        if (length >= SMS_BUFFER_SIZE - 1) return;
        for (int16_t i = length; i > (int16_t)cursorPos; i--) {
            text[i] = text[i - 1];
        }
        text[cursorPos] = c;
        cursorPos++;
        length++;
        text[length] = '\0';
    }
    
    void deleteChar(void) {
        if (cursorPos > 0) {
            cursorPos--;
            for (uint16_t i = cursorPos; i < length - 1; i++) {
                text[i] = text[i + 1];
            }
            length--;
            text[length] = '\0';
        }
    }
    
    void cursorLeft(void) { if (cursorPos > 0) cursorPos--; }
    void cursorRight(void) { if (cursorPos < length) cursorPos++; }
    void cursorHome(void) { cursorPos = 0; }
    void cursorEnd(void) { cursorPos = length; }
    
    const char* getText(void) const { return text; }
    uint16_t getLength(void) const { return length; }
    uint16_t getCursorPos(void) const { return cursorPos; }
    uint16_t getRemaining(void) const { return SMS_MAX_LENGTH - length; }
    bool isFull(void) const { return length >= SMS_MAX_LENGTH; }
    bool isEmpty(void) const { return length == 0; }
    
    void clear(void) {
        memset(text, 0, sizeof(text));
        length = 0;
        cursorPos = 0;
    }
    
    void setText(const char* newText) {
        if (newText) {
            strncpy(text, newText, SMS_BUFFER_SIZE - 1);
            text[SMS_BUFFER_SIZE - 1] = '\0';
            length = strlen(text);
            cursorPos = length;
        }
    }
};

// ============================================
// TYPES DE BUFFERS PREDEFINIS
// ============================================
typedef CircularBuffer<uint8_t, 16> ByteBuffer16;
typedef CircularBuffer<uint16_t, 8> WordBuffer8;
typedef CircularBuffer<char, 64> CharBuffer64;

#endif // BUFFERS_H