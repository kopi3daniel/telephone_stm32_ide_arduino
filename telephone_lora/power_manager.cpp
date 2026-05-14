

/**
 * ---------------------------------------------------------------------------
 * power_manager.cpp - Implementation de la gestion d energie
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "power_manager.h"
#include "display.h"        // <-- AJOUTER : pour Display
#include <stdio.h>

// Reference externe a l ecran (pour l eteindre)
extern Display display;

// ... reste du code ...

// Reference externe a l ecran (pour l eteindre)
extern Display display;

// ============================================
// CONSTRUCTEUR
// ============================================
PowerManager::PowerManager() {
    currentMode = POWER_MODE_ACTIVE;
    
    memset(&batteryInfo, 0, sizeof(batteryInfo));
    batteryInfo.percentage = 100;
    batteryInfo.state = BATTERY_NORMAL;
    
    screenOffTimeout = TIMEOUT_SCREEN_OFF_MS;
    sleepTimeout = TIMEOUT_SLEEP_MS;
    stopTimeout = TIMEOUT_STOP_MS;
    
    lastUserActivity = 0;
    sleepAttempts = 0;
    totalUptime = 0;
    timeInActive = 0;
    timeInSleep = 0;
    timeInStop = 0;
    lastModeChange = 0;
}

// ============================================
// INITIALISATION
// ============================================
void PowerManager::init(void) {
    
    // Configurer la broche ADC pour la batterie
    pinMode(BATTERY_ADC_PIN, INPUT_ANALOG);
    
    // Premiere mesure de batterie
    updateBatteryInfo();
    
    // Enregistrer le debut de fonctionnement
    lastUserActivity = millis();
    lastModeChange = millis();
    
    // Verifier si la batterie est suffisante pour demarrer
    if (isBatteryTooLow()) {
        emergencyShutdown();
    }
}

// ============================================
// BOUCLE PRINCIPALE
// ============================================
void PowerManager::process(void) {
    
    uint32_t now = millis();
    uint32_t idleTime = now - lastUserActivity;
    
    totalUptime = now / 1000;
    
    // Mettre a jour les statistiques de temps par mode
    uint32_t timeSinceChange = (now - lastModeChange) / 1000;
    switch (currentMode) {
        case POWER_MODE_ACTIVE:
        case POWER_MODE_SCREEN_OFF:
            timeInActive += timeSinceChange;
            break;
        case POWER_MODE_SLEEP:
            timeInSleep += timeSinceChange;
            break;
        case POWER_MODE_STOP:
            timeInStop += timeSinceChange;
            break;
        default:
            break;
    }
    lastModeChange = now;
    
    // Mesurer la batterie toutes les 30 secondes
    static uint32_t lastBatCheck = 0;
    if (now - lastBatCheck > 30000) {
        updateBatteryInfo();
        lastBatCheck = now;
        
        if (batteryInfo.state == BATTERY_EMERGENCY) {
            emergencyShutdown();
            return;
        }
    }
    
    // Verifier les transitions automatiques selon l inactivite
    switch (currentMode) {
        
        case POWER_MODE_ACTIVE:
            if (idleTime > screenOffTimeout) {
                enterScreenOff();
            }
            break;
        
        case POWER_MODE_SCREEN_OFF:
            if (idleTime > sleepTimeout) {
                enterSleep();
            }
            break;
        
        case POWER_MODE_SLEEP:
            if (idleTime > stopTimeout) {
                enterStop();
            }
            break;
        
        case POWER_MODE_STOP:
            break;
        
        case POWER_MODE_STANDBY:
            break;
    }
    
    // Alerte batterie faible
    if (batteryInfo.state == BATTERY_LOW && batteryInfo.percentage <= 15) {
        static uint32_t lastAlert = 0;
        if (now - lastAlert > 60000) {
            display.showLowBattery(batteryInfo.percentage);
            lastAlert = now;
        }
    }
}

// ============================================
// LECTURE DE LA TENSION BATTERIE
// ============================================
uint16_t PowerManager::readBatteryVoltage(void) {
    
    // Lire l'ADC (10 bits sur STM32F103 : 0-1023)
    int adcValue = analogRead(BATTERY_ADC_PIN);
    
    // Convertir en tension (mV)
    // Tension de reference = 3.3V = 3300mV
    // Resolution = 10 bits = 1024 niveaux
    uint16_t voltage_mV = (uint16_t)((uint32_t)adcValue * 3300 / 1023);
    
    // Appliquer le ratio du diviseur de tension
    // Si diviseur 2:1 (2 resistances egales), multiplier par 2
    // Ajuster selon votre circuit !
    voltage_mV = voltage_mV * 2;
    
    return voltage_mV;
}

// ============================================
// CONVERSION TENSION -> POURCENTAGE
// ============================================
uint8_t PowerManager::voltageToPercent(uint16_t mV) {
    
    if (mV >= BATTERY_FULL_MV) {
        return 100;
    } else if (mV <= 3000) {
        return 0;
    } else {
        // Interpolation lineaire
        return (uint8_t)(((mV - 3000UL) * 100) / 1200);
    }
}

// ============================================
// DETERMINER L ETAT DE LA BATTERIE
// ============================================
BatteryState_t PowerManager::determineBatteryState(uint8_t percent) {
    if (percent > 20)  return BATTERY_NORMAL;
    if (percent > 10)  return BATTERY_LOW;
    if (percent > 5)   return BATTERY_CRITICAL;
    return BATTERY_EMERGENCY;
}

// ============================================
// MISE A JOUR DES INFOS BATTERIE
// ============================================
void PowerManager::updateBatteryInfo(void) {
    batteryInfo.voltage_mV = readBatteryVoltage();
    batteryInfo.percentage = voltageToPercent(batteryInfo.voltage_mV);
    batteryInfo.state = determineBatteryState(batteryInfo.percentage);
    batteryInfo.lastMeasurement = millis();
    
    // Detection de charge
    static uint16_t lastVoltage = 0;
    if (batteryInfo.voltage_mV > lastVoltage + 50) {
        batteryInfo.charging = true;
    } else if (batteryInfo.voltage_mV > 4100) {
        batteryInfo.charging = true;
    } else {
        batteryInfo.charging = false;
    }
    lastVoltage = batteryInfo.voltage_mV;
}

// ============================================
// PASSAGE EN MODE SCREEN OFF
// ============================================
void PowerManager::enterScreenOff(void) {
    if (currentMode != POWER_MODE_ACTIVE) return;
    
    currentMode = POWER_MODE_SCREEN_OFF;
    display.powerOff();
}

// ============================================
// PASSAGE EN MODE SLEEP (economie legere)
// ============================================
void PowerManager::enterSleep(void) {
    if (currentMode == POWER_MODE_STOP || 
        currentMode == POWER_MODE_STANDBY) return;
    
    currentMode = POWER_MODE_SLEEP;
    
    display.powerOff();
    disableUnusedPeripherals();
    
    // Sur Arduino, on ne peut pas facilement changer la frequence CPU
    // On se contente de desactiver les peripheriques
}

// ============================================
// PASSAGE EN MODE STOP (economie profonde)
// ============================================
void PowerManager::enterStop(void) {
    if (currentMode == POWER_MODE_STANDBY) return;
    
    currentMode = POWER_MODE_STOP;
    
    display.powerOff();
    disableUnusedPeripherals();
    
    // Sur Arduino, le mode STOP n'est pas directement accessible
    // On utilise un delai long avec interruptions desactivees
    // Pour un vrai mode STOP, il faut utiliser la bibliothèque STM32LowPower
    
    // Simulation : attendre avec faible consommation
    delay(1000);  // Attendre 1 seconde (sera repete par la boucle)
}

// ============================================
// PASSAGE EN MODE STANDBY (extinction quasi-totale)
// ============================================
void PowerManager::enterStandby(void) {
    
    display.showAlert("Extinction", "Batterie epuisee");
    delay(1000);
    
    display.powerOff();
    disableUnusedPeripherals();
    
    currentMode = POWER_MODE_STANDBY;
    
    // Mettre toutes les broches en entree pour economiser
    for (int i = 0; i < 48; i++) {
        pinMode(i, INPUT);
    }
    
    // Boucle infinie (simule l'arret)
    while (1) {
        delay(1000);
    }
}

// ============================================
// REVEIL
// ============================================
void PowerManager::wakeUp(void) {
    
    if (currentMode == POWER_MODE_SLEEP || 
        currentMode == POWER_MODE_STOP) {
        enableAllPeripherals();
    }
    
    currentMode = POWER_MODE_ACTIVE;
    lastUserActivity = millis();
    
    display.powerOn();
}

// ============================================
// ACTIVITE UTILISATEUR
// ============================================
void PowerManager::userActivity(void) {
    lastUserActivity = millis();
    
    if (currentMode == POWER_MODE_SLEEP || 
        currentMode == POWER_MODE_SCREEN_OFF ||
        currentMode == POWER_MODE_STOP) {
        wakeUp();
    }
}

// ============================================
// GESTION DES PERIPHERIQUES
// ============================================
void PowerManager::disableUnusedPeripherals(void) {
    // Desactiver les pins PWM (mettre en entree)
    pinMode(AUDIO_SPK_PIN, INPUT);
    pinMode(TORCH_PIN, INPUT);
    
    // Arreter les timers logiciels
    noTone(AUDIO_SPK_PIN);
    noTone(BUZZER_PIN);
}

void PowerManager::enableAllPeripherals(void) {
    // Reactiver les pins
    pinMode(AUDIO_SPK_PIN, OUTPUT);
    pinMode(TORCH_PIN, OUTPUT);
}

// ============================================
// INFORMATIONS
// ============================================
BatteryInfo_t PowerManager::getBatteryInfo(void) {
    return batteryInfo;
}

uint8_t PowerManager::getBatteryPercent(void) {
    return batteryInfo.percentage;
}

uint16_t PowerManager::getBatteryVoltage(void) {
    return batteryInfo.voltage_mV;
}

bool PowerManager::isCharging(void) {
    return batteryInfo.charging;
}

PowerMode_t PowerManager::getMode(void) {
    return currentMode;
}

const char* PowerManager::getModeName(void) {
    switch (currentMode) {
        case POWER_MODE_ACTIVE:     return "ACTIF";
        case POWER_MODE_SCREEN_OFF: return "ECO";
        case POWER_MODE_SLEEP:      return "VEILLE";
        case POWER_MODE_STOP:       return "PROFOND";
        case POWER_MODE_STANDBY:    return "ETEINT";
        default:                    return "???";
    }
}

// ============================================
// ESTIMATION AUTONOMIE
// ============================================
uint32_t PowerManager::estimateRemainingTime(void) {
    return estimateTimeForMode(currentMode);
}

uint32_t PowerManager::estimateTimeForMode(PowerMode_t mode) {
    const uint16_t batteryCapacity = 2000;  // mAh
    
    uint16_t consumption;
    switch (mode) {
        case POWER_MODE_ACTIVE:     consumption = POWER_ACTIVE_MA;     break;
        case POWER_MODE_SCREEN_OFF: consumption = 80;                   break;
        case POWER_MODE_SLEEP:      consumption = POWER_SLEEP_MA;      break;
        case POWER_MODE_STOP:       consumption = POWER_STOP_MA;       break;
        case POWER_MODE_STANDBY:    consumption = 0;                    break;
        default:                    consumption = POWER_ACTIVE_MA;     break;
    }
    
    float remainingCapacity = batteryCapacity * (batteryInfo.percentage / 100.0f);
    float hoursRemaining = remainingCapacity / consumption;
    
    return (uint32_t)(hoursRemaining * 60);
}

// ============================================
// STATISTIQUES
// ============================================
uint32_t PowerManager::getUptime(void) {
    return totalUptime;
}

uint16_t PowerManager::getAverageConsumption(void) {
    if (totalUptime == 0) return 0;
    
    uint32_t totalConsumption = 
        (timeInActive * POWER_ACTIVE_MA) + 
        (timeInSleep * POWER_SLEEP_MA) + 
        (timeInStop * POWER_STOP_MA);
    
    return (uint16_t)(totalConsumption / totalUptime);
}

// ============================================
// PROTECTION
// ============================================
bool PowerManager::isBatteryTooLow(void) {
    return (batteryInfo.state == BATTERY_EMERGENCY && 
            batteryInfo.percentage < 3 &&
            !batteryInfo.charging);
}

void PowerManager::emergencyShutdown(void) {
    
    display.showAlert("ARRET", "Batterie critique");
    delay(2000);
    
    enterStandby();
}

// ============================================
// FIN DU FICHIER power_manager.cpp
// ============================================