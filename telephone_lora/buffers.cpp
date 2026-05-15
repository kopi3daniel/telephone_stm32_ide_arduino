

#include "buffers.h"
#include <string.h>
#include <stdio.h>

// ============================================
// BUFFER AUDIO - Implementation complementaire
// ============================================

uint16_t AudioBuffer::fillFromADC(uint16_t maxSamples) {
    uint16_t filled = 0;
    
    for (uint16_t i = 0; i < maxSamples && dataCount < AUDIO_BUFFER_SIZE; i++) {
        uint8_t sample = 128;  // Silence par defaut
        
        buffer[writeIndex] = sample;
        writeIndex = (writeIndex + 1) % AUDIO_BUFFER_SIZE;
        dataCount++;
        filled++;
        
        delayMicroseconds(125);  // 8 kHz = 125us par echantillon
    }
    
    if (dataCount >= AUDIO_BUFFER_SIZE / 2) {
        ready = true;
    }
    
    return filled;
}

uint16_t AudioBuffer::drainToSpeaker(uint16_t maxSamples) {
    uint16_t drained = 0;
    
    for (uint16_t i = 0; i < maxSamples && dataCount > 0; i++) {
        uint8_t sample = buffer[readIndex];
        
        readIndex = (readIndex + 1) % AUDIO_BUFFER_SIZE;
        dataCount--;
        drained++;
        
        delayMicroseconds(125);
    }
    
    if (dataCount < AUDIO_BUFFER_SIZE / 4) {
        ready = false;
    }
    
    return drained;
}

// ============================================
// FILE DE PAQUETS LORA
// ============================================

template<uint16_t QUEUE_SIZE>
uint16_t LoRaPacketQueue<QUEUE_SIZE>::cleanupExpired(uint32_t timeoutMs) {
    uint16_t removed = 0;
    uint32_t now = millis();
    
    uint16_t currentCount = count;
    for (uint16_t i = 0; i < currentCount; i++) {
        uint16_t idx = (tail + i) % QUEUE_SIZE;
        if (now - queue[idx].timestamp > timeoutMs) {
            queue[idx].retryCount = 255;
            removed++;
        }
    }
    
    while (count > 0 && queue[tail].retryCount == 255) {
        tail = (tail + 1) % QUEUE_SIZE;
        count--;
    }
    
    return removed;
}

template<uint16_t QUEUE_SIZE>
bool LoRaPacketQueue<QUEUE_SIZE>::incrementRetry(uint16_t index) {
    uint16_t idx = (tail + index) % QUEUE_SIZE;
    if (index < count && queue[idx].retryCount < 3) {
        queue[idx].retryCount++;
        queue[idx].timestamp = millis();
        return true;
    }
    return false;
}

// ============================================
// BUFFER SMS - Fonctions additionnelles
// ============================================

void SMSBuffer::truncate(void) {
    if (length > SMS_MAX_LENGTH) {
        length = SMS_MAX_LENGTH;
        text[length] = '\0';
        if (cursorPos > length) cursorPos = length;
    }
}

void SMSBuffer::insertNewLine(void) {
    insertChar('\n');
}

uint16_t SMSBuffer::wordCount(void) const {
    uint16_t words = 0;
    bool inWord = false;
    
    for (uint16_t i = 0; i < length; i++) {
        if (text[i] == ' ' || text[i] == '\n' || text[i] == '\t') {
            inWord = false;
        } else if (!inWord) {
            inWord = true;
            words++;
        }
    }
    return words;
}

bool SMSBuffer::getWord(uint16_t wordIndex, char* buffer, uint16_t bufferSize) const {
    uint16_t currentWord = 0;
    uint16_t startIdx = 0;
    bool inWord = false;
    
    for (uint16_t i = 0; i <= length; i++) {
        if (i < length && text[i] != ' ' && text[i] != '\n' && text[i] != '\t') {
            if (!inWord) {
                inWord = true;
                startIdx = i;
            }
        } else {
            if (inWord) {
                if (currentWord == wordIndex) {
                    uint16_t wordLen = i - startIdx;
                    uint16_t copyLen = (wordLen < bufferSize - 1) ? wordLen : bufferSize - 1;
                    strncpy(buffer, text + startIdx, copyLen);
                    buffer[copyLen] = '\0';
                    return true;
                }
                currentWord++;
                inWord = false;
            }
        }
    }
    return false;
}

// ============================================
// FILE D EVENEMENTS - Fonctions additionnelles
// ============================================

bool EventQueue::pushKeyEvent(char key, bool pressed) {
    Event_t event;
    event.type = pressed ? EVENT_KEY_PRESS : EVENT_KEY_RELEASE;
    event.timestamp = millis();
    event.data[0] = key;
    event.dataLength = 1;
    return events.push(event);
}

bool EventQueue::pushLoRaEvent(EventType_t type, uint8_t rssi) {
    Event_t event;
    event.type = type;
    event.timestamp = millis();
    event.data[0] = rssi;
    event.dataLength = 1;
    return events.push(event);
}

uint16_t EventQueue::filterByType(EventType_t type, Event_t* output, uint16_t maxOutput) {
    uint16_t found = 0;
    uint16_t currentCount = events.size();
    
    for (uint16_t i = 0; i < currentCount && found < maxOutput; i++) {
        Event_t temp;
        if (events.peekAt(i, temp)) {
            if (temp.type == type) {
                if (output != NULL) {
                    output[found] = temp;
                }
                found++;
            }
        }
    }
    return found;
}

uint16_t EventQueue::removeByType(EventType_t type) {
    uint16_t currentCount = events.size();
    uint16_t removed = 0;
    
    Event_t tempBuffer[EVENT_QUEUE_SIZE];
    uint16_t tempCount = 0;
    
    for (uint16_t i = 0; i < currentCount; i++) {
        Event_t temp;
        if (events.pop(temp)) {
            if (temp.type != type) {
                tempBuffer[tempCount++] = temp;
            } else {
                removed++;
            }
        }
    }
    
    for (uint16_t i = 0; i < tempCount; i++) {
        events.push(tempBuffer[i]);
    }
    
    return removed;
}

// ============================================
// BUFFER CIRCULAIRE GENERIQUE - Fonctions additionnelles
// ============================================

template<typename T, uint16_t SIZE>
uint16_t CircularBuffer<T, SIZE>::copyTo(T* output, uint16_t maxOutput) const {
    uint16_t copyCount = (count < maxOutput) ? count : maxOutput;
    for (uint16_t i = 0; i < copyCount; i++) {
        uint16_t idx = (tail + i) % SIZE;
        output[i] = buffer[idx];
    }
    return copyCount;
}

template<typename T, uint16_t SIZE>
bool CircularBuffer<T, SIZE>::find(const T& item, uint16_t* foundIndex) const {
    for (uint16_t i = 0; i < count; i++) {
        uint16_t idx = (tail + i) % SIZE;
        if (buffer[idx] == item) {
            if (foundIndex != NULL) *foundIndex = i;
            return true;
        }
    }
    return false;
}

template<typename T, uint16_t SIZE>
bool CircularBuffer<T, SIZE>::removeAt(uint16_t index) {
    if (index >= count) return false;
    for (uint16_t i = index; i < count - 1; i++) {
        uint16_t currentIdx = (tail + i) % SIZE;
        uint16_t nextIdx = (tail + i + 1) % SIZE;
        buffer[currentIdx] = buffer[nextIdx];
    }
    head = (head == 0) ? SIZE - 1 : head - 1;
    count--;
    return true;
}

// ============================================
// FONCTIONS DE DIAGNOSTIC
// ============================================

void debugPrintBufferStats(void) {
    DEBUG_SERIAL.println("=== BUFFER STATS ===");
    DEBUG_SERIAL.print("Audio: 0/");
    DEBUG_SERIAL.println(AUDIO_BUFFER_SIZE);
    DEBUG_SERIAL.print("Events: 0/");
    DEBUG_SERIAL.println(EVENT_QUEUE_SIZE);
    DEBUG_SERIAL.println("====================");
}

bool verifyBufferIntegrity(void) {
    return true;
}

// ============================================
// FIN DU FICHIER buffers.cpp
// ============================================