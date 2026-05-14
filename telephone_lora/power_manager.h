/**
 * ---------------------------------------------------------------------------
 * power_manager.h - Gestion de l energie et de la batterie
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Ce fichier declare la classe PowerManager qui gere :
 * - La mesure de la tension batterie via ADC
 * - Le calcul du pourcentage restant
 * - Les modes d economie d energie (RUN, SLEEP, STOP, STANDBY)
 * - Les transitions automatiques selon l inactivite
 * - L estimation de l autonomie restante
 * - La protection contre la decharge profonde
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "config.h"

// ============================================
// MODES D ENERGIE DU SYSTEME
// ============================================
typedef enum {
    POWER_MODE_ACTIVE,      // Tout allume : CPU 72MHz, ecran ON, LoRa RX
    POWER_MODE_SCREEN_OFF,  // Ecran eteint, CPU normal, LoRa RX
    POWER_MODE_SLEEP,       // CPU ralenti, ecran OFF, LoRa cyclique
    POWER_MODE_STOP,        // CPU stoppe, RTC actif, LoRa rare
    POWER_MODE_STANDBY      // Tout eteint sauf wakeup pin
} PowerMode_t;

// ============================================
// ETATS DE LA BATTERIE
// ============================================
typedef enum {
    BATTERY_NORMAL,         // Niveau normal (> 20%)
    BATTERY_LOW,            // Batterie faible (10-20%)
    BATTERY_CRITICAL,       // Batterie critique (5-10%)
    BATTERY_EMERGENCY       // Extinction imminente (< 5%)
} BatteryState_t;

// ============================================
// STRUCTURE D INFORMATION BATTERIE
// ============================================
typedef struct {
    uint16_t voltage_mV;        // Tension actuelle en millivolts
    uint8_t  percentage;        // Pourcentage restant (0-100)
    BatteryState_t state;       // Etat actuel
    bool     charging;          // En charge ?
    uint32_t lastMeasurement;   // Timestamp derniere mesure
} BatteryInfo_t;

// ============================================
// CLASSE PRINCIPALE
// ============================================
class PowerManager {
    
private:
    // ============================================
    // ETAT ACTUEL
    // ============================================
    PowerMode_t currentMode;        // Mode d energie actuel
    BatteryInfo_t batteryInfo;      // Informations batterie
    uint32_t lastUserActivity;      // Derniere interaction utilisateur
    uint8_t  sleepAttempts;         // Tentatives de mise en veille
    
    // ============================================
    // PARAMETRES DE CONFIGURATION
    // ============================================
    uint32_t screenOffTimeout;      // Delai avant extinction ecran (ms)
    uint32_t sleepTimeout;          // Delai avant mode SLEEP (ms)
    uint32_t stopTimeout;           // Delai avant mode STOP (ms)
    
    // ============================================
    // SUIVI DE LA CONSOMMATION
    // ============================================
    uint32_t totalUptime;           // Temps total de fonctionnement (secondes)
    uint32_t timeInActive;          // Temps passe en mode ACTIF
    uint32_t timeInSleep;           // Temps passe en SLEEP
    uint32_t timeInStop;            // Temps passe en STOP
    uint32_t lastModeChange;        // Dernier changement de mode
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Lit la tension batterie via ADC
    uint16_t readBatteryVoltage(void);
    
    // Calcule le pourcentage a partir de la tension
    uint8_t voltageToPercent(uint16_t mV);
    
    // Determine l etat de la batterie
    BatteryState_t determineBatteryState(uint8_t percent);
    
    // Desactive les peripheriques inutiles (LED, PWM, etc.)
    void disableUnusedPeripherals(void);
    
    // Reactive les peripheriques
    void enableAllPeripherals(void);
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    PowerManager();
    
    // Initialise le gestionnaire d energie
    void init(void);
    
    // ============================================
    // BOUCLE PRINCIPALE (a appeler regulierement)
    // ============================================
    void process(void);
    
    // ============================================
    // CONTROLE DES MODES
    // ============================================
    
    // Passe en mode ACTIF (tout allume)
    void enterActive(void);
    
    // Eteint seulement l ecran
    void enterScreenOff(void);
    
    // Passe en mode SLEEP (economie legere)
    void enterSleep(void);
    
    // Passe en mode STOP (economie profonde)
    void enterStop(void);
    
    // Passe en mode STANDBY (extinction quasi-totale)
    void enterStandby(void);
    
    // Reveil depuis SLEEP ou STOP
    void wakeUp(void);
    
    // ============================================
    // ACTIVITE UTILISATEUR
    // ============================================
    
    // Signale une activite utilisateur (reset les timeouts)
    void userActivity(void);
    
    // ============================================
    // INFORMATIONS BATTERIE
    // ============================================
    
    // Mesure et met a jour les infos batterie
    void updateBatteryInfo(void);
    
    // Retourne les infos batterie
    BatteryInfo_t getBatteryInfo(void);
    
    // Retourne le pourcentage (0-100)
    uint8_t getBatteryPercent(void);
    
    // Retourne la tension en mV
    uint16_t getBatteryVoltage(void);
    
    // Verifie si la batterie est en charge
    bool isCharging(void);
    
    // ============================================
    // MODE ACTUEL
    // ============================================
    
    // Retourne le mode d energie actuel
    PowerMode_t getMode(void);
    
    // Retourne le nom du mode actuel (pour affichage)
    const char* getModeName(void);
    
    // ============================================
    // ESTIMATION AUTONOMIE
    // ============================================
    
    // Estime le temps restant en minutes selon le mode actuel
    uint32_t estimateRemainingTime(void);
    
    // Estime l autonomie en heures pour un mode donne
    uint32_t estimateTimeForMode(PowerMode_t mode);
    
    // ============================================
    // STATISTIQUES
    // ============================================
    
    // Retourne le temps total de fonctionnement
    uint32_t getUptime(void);
    
    // Retourne la consommation moyenne estimee (mA)
    uint16_t getAverageConsumption(void);
    
    // ============================================
    // PROTECTION
    // ============================================
    
    // Verifie si la batterie est trop faible pour fonctionner
    bool isBatteryTooLow(void);
    
    // Force l extinction si batterie critique
    void emergencyShutdown(void);
};

#endif // POWER_MANAGER_H