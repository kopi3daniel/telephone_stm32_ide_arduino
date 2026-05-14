/**
 * ---------------------------------------------------------------------------
 * timers.cpp - Implementation des timers logiciels
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "timers.h"
#include <string.h>
#include <stdio.h>

// ============================================
// CONSTRUCTEUR
// ============================================
TimerManager::TimerManager() {
    timerCount = 0;
    nextTimerId = 1;
    systemTick = 0;
    startupTick = 0;
    totalMilliseconds = 0;
    
    memset(timers, 0, sizeof(timers));
}

void TimerManager::init(void) {
    startupTick = millis();
    systemTick = startupTick;
    timerCount = 0;
    nextTimerId = 1;
}

// ============================================
// MISE A JOUR PRINCIPALE
// ============================================
void TimerManager::update(void) {
    systemTick = millis();
    totalMilliseconds = (uint64_t)(systemTick - startupTick);
    
    for (uint8_t i = 0; i < MAX_SOFT_TIMERS; i++) {
        SoftTimer_t* timer = &timers[i];
        
        if (timer->state == TIMER_STATE_IDLE || 
            timer->state == TIMER_STATE_PAUSED ||
            timer->state == TIMER_STATE_EXPIRED) {
            continue;
        }
        
        uint32_t elapsed = systemTick - timer->startTime;
        
        if (elapsed >= timer->interval) {
            
            if (timer->callback != NULL) {
                timer->callback(timer->callbackParam);
            }
            
            timer->triggerCount++;
            timer->lastTrigger = systemTick;
            
            switch (timer->type) {
                case TIMER_TYPE_ONESHOT:
                    timer->state = TIMER_STATE_EXPIRED;
                    break;
                    
                case TIMER_TYPE_PERIODIC:
                    timer->startTime = systemTick;
                    timer->remaining = timer->interval;
                    break;
                    
                case TIMER_TYPE_WATCHDOG:
                    timer->state = TIMER_STATE_EXPIRED;
                    break;
            }
        } else {
            timer->remaining = timer->interval - elapsed;
        }
    }
}

// ============================================
// RECHERCHE DE TIMERS
// ============================================
int8_t TimerManager::findTimerById(uint8_t id) {
    for (uint8_t i = 0; i < MAX_SOFT_TIMERS; i++) {
        if (timers[i].id == id && timers[i].state != TIMER_STATE_IDLE) {
            return i;
        }
    }
    return -1;
}

int8_t TimerManager::findTimerByName(const char* name) {
    if (name == NULL) return -1;
    for (uint8_t i = 0; i < MAX_SOFT_TIMERS; i++) {
        if (timers[i].name != NULL && strcmp(timers[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int8_t TimerManager::findFreeSlot(void) {
    for (uint8_t i = 0; i < MAX_SOFT_TIMERS; i++) {
        if (timers[i].state == TIMER_STATE_IDLE) {
            return i;
        }
    }
    return -1;
}

// ============================================
// CREATION DE TIMERS
// ============================================
uint8_t TimerManager::createOneShot(const char* name, uint32_t timeoutMs,
                                   void (*callback)(void*), void* param) {
    int8_t slot = findFreeSlot();
    if (slot < 0) return 0;
    
    SoftTimer_t* timer = &timers[slot];
    timer->id = nextTimerId++;
    timer->name = name;
    timer->type = TIMER_TYPE_ONESHOT;
    timer->state = TIMER_STATE_IDLE;
    timer->interval = timeoutMs;
    timer->startTime = 0;
    timer->lastTrigger = 0;
    timer->remaining = timeoutMs;
    timer->callback = callback;
    timer->callbackParam = param;
    timer->autoReload = false;
    timer->triggerCount = 0;
    
    timerCount++;
    return timer->id;
}

uint8_t TimerManager::createPeriodic(const char* name, uint32_t intervalMs,
                                    void (*callback)(void*), void* param) {
    int8_t slot = findFreeSlot();
    if (slot < 0) return 0;
    
    SoftTimer_t* timer = &timers[slot];
    timer->id = nextTimerId++;
    timer->name = name;
    timer->type = TIMER_TYPE_PERIODIC;
    timer->state = TIMER_STATE_IDLE;
    timer->interval = intervalMs;
    timer->startTime = 0;
    timer->lastTrigger = 0;
    timer->remaining = intervalMs;
    timer->callback = callback;
    timer->callbackParam = param;
    timer->autoReload = true;
    timer->triggerCount = 0;
    
    timerCount++;
    return timer->id;
}

uint8_t TimerManager::createWatchdog(const char* name, uint32_t timeoutMs,
                                    void (*callback)(void*), void* param) {
    int8_t slot = findFreeSlot();
    if (slot < 0) return 0;
    
    SoftTimer_t* timer = &timers[slot];
    timer->id = nextTimerId++;
    timer->name = name;
    timer->type = TIMER_TYPE_WATCHDOG;
    timer->state = TIMER_STATE_IDLE;
    timer->interval = timeoutMs;
    timer->startTime = 0;
    timer->lastTrigger = 0;
    timer->remaining = timeoutMs;
    timer->callback = callback;
    timer->callbackParam = param;
    timer->autoReload = false;
    timer->triggerCount = 0;
    
    timerCount++;
    return timer->id;
}

// ============================================
// CONTROLE DES TIMERS
// ============================================
bool TimerManager::startTimer(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return false;
    
    SoftTimer_t* timer = &timers[index];
    timer->startTime = systemTick;
    timer->remaining = timer->interval;
    timer->state = TIMER_STATE_RUNNING;
    
    return true;
}

bool TimerManager::stopTimer(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return false;
    
    timers[index].state = TIMER_STATE_IDLE;
    timerCount--;
    
    return true;
}

bool TimerManager::pauseTimer(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return false;
    if (timers[index].state != TIMER_STATE_RUNNING) return false;
    
    timers[index].remaining = timers[index].interval - 
                             (systemTick - timers[index].startTime);
    timers[index].state = TIMER_STATE_PAUSED;
    
    return true;
}

bool TimerManager::resumeTimer(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return false;
    if (timers[index].state != TIMER_STATE_PAUSED) return false;
    
    timers[index].interval = timers[index].remaining;
    timers[index].startTime = systemTick;
    timers[index].state = TIMER_STATE_RUNNING;
    
    return true;
}

bool TimerManager::resetTimer(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return false;
    
    timers[index].startTime = systemTick;
    timers[index].remaining = timers[index].interval;
    timers[index].state = TIMER_STATE_RUNNING;
    
    return true;
}

bool TimerManager::deleteTimer(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return false;
    
    memset(&timers[index], 0, sizeof(SoftTimer_t));
    timers[index].state = TIMER_STATE_IDLE;
    timerCount--;
    
    return true;
}

// ============================================
// MODIFICATION
// ============================================
bool TimerManager::setTimerInterval(uint8_t id, uint32_t newInterval) {
    int8_t index = findTimerById(id);
    if (index < 0) return false;
    
    timers[index].interval = newInterval;
    return true;
}

bool TimerManager::setTimerCallback(uint8_t id, void (*callback)(void*), void* param) {
    int8_t index = findTimerById(id);
    if (index < 0) return false;
    
    timers[index].callback = callback;
    timers[index].callbackParam = param;
    return true;
}

// ============================================
// INFORMATIONS
// ============================================
bool TimerManager::hasExpired(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return true;
    
    return (timers[index].state == TIMER_STATE_EXPIRED);
}

uint32_t TimerManager::getElapsedTime(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return 0;
    if (timers[index].state != TIMER_STATE_RUNNING) return 0;
    
    return systemTick - timers[index].startTime;
}

uint32_t TimerManager::getRemainingTime(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return 0;
    
    if (timers[index].state == TIMER_STATE_RUNNING) {
        uint32_t elapsed = systemTick - timers[index].startTime;
        return (elapsed >= timers[index].interval) ? 0 : timers[index].interval - elapsed;
    }
    return timers[index].remaining;
}

TimerState_t TimerManager::getTimerState(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return TIMER_STATE_IDLE;
    return timers[index].state;
}

uint32_t TimerManager::getTriggerCount(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return 0;
    return timers[index].triggerCount;
}

uint8_t TimerManager::getActiveTimerCount(void) { return timerCount; }

// ============================================
// HORLOGE SYSTEME
// ============================================
uint32_t TimerManager::getTick(void) { return systemTick; }
uint64_t TimerManager::getUptimeMs(void) { return totalMilliseconds; }
uint32_t TimerManager::getUptimeSec(void) { return (uint32_t)(totalMilliseconds / 1000); }
uint32_t TimerManager::msToTicks(uint32_t ms) { return ms; }
uint32_t TimerManager::secToMs(uint32_t sec) { return sec * 1000; }

// ============================================
// DELAI NON-BLOQUANT
// ============================================
bool TimerManager::delayElapsed(uint32_t startTick, uint32_t delayMs) {
    return (systemTick - startTick) >= delayMs;
}

bool TimerManager::waitForCondition(bool (*condition)(void*), void* param, uint32_t timeoutMs) {
    uint32_t start = systemTick;
    
    while (!delayElapsed(start, timeoutMs)) {
        if (condition != NULL && condition(param)) {
            return true;
        }
        delay(1);  // Remplacer __WFI() par delay(1)
    }
    
    return false;
}

// ============================================
// DEBOGAGE
// ============================================
void TimerManager::debugPrintAll(void) {
    DEBUG_SERIAL.println("=== TIMERS ===");
    for (uint8_t i = 0; i < MAX_SOFT_TIMERS; i++) {
        if (timers[i].state != TIMER_STATE_IDLE) {
            DEBUG_SERIAL.print("  [");
            DEBUG_SERIAL.print(timers[i].id);
            DEBUG_SERIAL.print("] ");
            DEBUG_SERIAL.print(timers[i].name ? timers[i].name : "?");
            DEBUG_SERIAL.print(" : ");
            switch (timers[i].state) {
                case TIMER_STATE_RUNNING:  DEBUG_SERIAL.print("RUNNING"); break;
                case TIMER_STATE_EXPIRED:  DEBUG_SERIAL.print("EXPIRED"); break;
                case TIMER_STATE_PAUSED:   DEBUG_SERIAL.print("PAUSED");  break;
                default:                   DEBUG_SERIAL.print("???");     break;
            }
            DEBUG_SERIAL.print(" (");
            DEBUG_SERIAL.print(getElapsedTime(timers[i].id));
            DEBUG_SERIAL.print("/");
            DEBUG_SERIAL.print(timers[i].interval);
            DEBUG_SERIAL.println("ms)");
        }
    }
    DEBUG_SERIAL.println("==============");
}

const char* TimerManager::getTimerName(uint8_t id) {
    int8_t index = findTimerById(id);
    if (index < 0) return NULL;
    return timers[index].name;
}

// ============================================
// FIN DU FICHIER timers.cpp
// ============================================