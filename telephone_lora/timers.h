/**
 * ---------------------------------------------------------------------------
 * timers.h - Gestion des timers logiciels pour le Telephone LoRa
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Ce module fournit des timers logiciels (non bloquants) pour :
 * - Les timeouts (appel, saisie, veille...)
 * - Les delais periodiques
 * - Les mesures de duree
 * - L horloge systeme (basee sur millis)
 * 
 * Les timers sont geres en logiciel (pas de hardware timer)
 * pour economiser les ressources du STM32.
 */

#ifndef TIMERS_H
#define TIMERS_H

#include <Arduino.h>
#include "config.h"

// ============================================
// NOMBRE MAXIMUM DE TIMERS SIMULTANES
// ============================================
#define MAX_SOFT_TIMERS         10

// ============================================
// TYPES DE TIMERS
// ============================================
typedef enum {
    TIMER_TYPE_ONESHOT,         // Se declenche une seule fois
    TIMER_TYPE_PERIODIC,        // Se declenche periodiquement
    TIMER_TYPE_WATCHDOG         // Doit etre rearme avant timeout
} TimerType_t;

// ============================================
// ETATS D UN TIMER
// ============================================
typedef enum {
    TIMER_STATE_IDLE,           // Inactif
    TIMER_STATE_RUNNING,        // En cours
    TIMER_STATE_EXPIRED,        // A expire (pour ONESHOT)
    TIMER_STATE_PAUSED          // En pause
} TimerState_t;

// ============================================
// STRUCTURE D UN TIMER LOGICIEL
// ============================================
typedef struct {
    uint8_t id;                 // Identifiant unique
    const char* name;           // Nom descriptif
    TimerType_t type;           // Type de timer
    TimerState_t state;         // Etat actuel
    uint32_t interval;          // Intervalle en millisecondes
    uint32_t startTime;         // Tick de demarrage
    uint32_t lastTrigger;       // Dernier declenchement
    uint32_t remaining;         // Temps restant (si en pause)
    void (*callback)(void*);    // Fonction a appeler
    void* callbackParam;        // Parametre pour le callback
    bool autoReload;            // Recharger automatiquement ?
    uint32_t triggerCount;      // Nombre de declenchements
} SoftTimer_t;

// ============================================
// CLASSE TIMER MANAGER
// ============================================
class TimerManager {
    
private:
    // ============================================
    // TABLE DES TIMERS
    // ============================================
    SoftTimer_t timers[MAX_SOFT_TIMERS];
    uint8_t timerCount;
    uint8_t nextTimerId;
    
    // ============================================
    // HORLOGE SYSTEME
    // ============================================
    uint32_t systemTick;
    uint32_t startupTick;
    uint64_t totalMilliseconds;
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    int8_t findTimerById(uint8_t id);
    int8_t findTimerByName(const char* name);
    int8_t findFreeSlot(void);
    
public:
    // ============================================
    // CONSTRUCTEUR
    // ============================================
    TimerManager();
    
    // ============================================
    // INITIALISATION
    // ============================================
    void init(void);
    
    // ============================================
    // MISE A JOUR
    // ============================================
    void update(void);
    
    // ============================================
    // CREATION DE TIMERS
    // ============================================
    
    uint8_t createOneShot(const char* name, uint32_t timeoutMs, 
                         void (*callback)(void*), void* param);
    
    uint8_t createPeriodic(const char* name, uint32_t intervalMs,
                          void (*callback)(void*), void* param);
    
    uint8_t createWatchdog(const char* name, uint32_t timeoutMs,
                          void (*callback)(void*), void* param);
    
    // ============================================
    // CONTROLE DES TIMERS
    // ============================================
    
    bool startTimer(uint8_t id);
    bool stopTimer(uint8_t id);
    bool pauseTimer(uint8_t id);
    bool resumeTimer(uint8_t id);
    bool resetTimer(uint8_t id);
    bool deleteTimer(uint8_t id);
    
    // ============================================
    // CHANGEMENT D INTERVALLE
    // ============================================
    
    bool setTimerInterval(uint8_t id, uint32_t newInterval);
    bool setTimerCallback(uint8_t id, void (*callback)(void*), void* param);
    
    // ============================================
    // INFORMATIONS
    // ============================================
    
    bool hasExpired(uint8_t id);
    uint32_t getElapsedTime(uint8_t id);
    uint32_t getRemainingTime(uint8_t id);
    TimerState_t getTimerState(uint8_t id);
    uint32_t getTriggerCount(uint8_t id);
    uint8_t getActiveTimerCount(void);
    
    // ============================================
    // HORLOGE SYSTEME
    // ============================================
    
    uint32_t getTick(void);
    uint64_t getUptimeMs(void);
    uint32_t getUptimeSec(void);
    uint32_t msToTicks(uint32_t ms);
    uint32_t secToMs(uint32_t sec);
    
    // ============================================
    // FONCTIONS DE DELAI NON-BLOQUANT
    // ============================================
    
    bool delayElapsed(uint32_t startTick, uint32_t delayMs);
    bool waitForCondition(bool (*condition)(void*), void* param, uint32_t timeoutMs);
    
    // ============================================
    // DEBOGAGE
    // ============================================
    
    void debugPrintAll(void);
    const char* getTimerName(uint8_t id);
};

// ============================================
// TIMERS PREDEFINIS POUR LE TELEPHONE
// ============================================
#define TIMER_CALL_RING         1
#define TIMER_SCREEN_OFF        2
#define TIMER_SLEEP_MODE        3
#define TIMER_VOICE_TIMEOUT     4
#define TIMER_SMS_TIMEOUT       5
#define TIMER_PRESENCE          6
#define TIMER_BATTERY_CHECK     7
#define TIMER_CURSOR_BLINK      8
#define TIMER_DIALOG_TIMEOUT    9

#endif // TIMERS_H