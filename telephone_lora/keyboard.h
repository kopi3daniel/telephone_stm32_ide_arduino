
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <Arduino.h>
#include "config.h"

// Modes de saisie
typedef enum {
    INPUT_MODE_LOWERCASE = 0,
    INPUT_MODE_UPPERCASE,
    INPUT_MODE_NUMBERS,
    INPUT_MODE_SYMBOLS
} InputMode_t;

class Keyboard {
    
public:
    Keyboard();
    void init(void);
    
    // Scan du clavier
    char scanKey(void);
    
    // Gestion multi-tap
    char multiTapInput(char key);
    char confirmMultiTap(void);
    void checkMultiTapTimeout(void);
    
    // Gestion des modes
    void toggleMode(void);
    void toggleShift(void);
    void setMode(InputMode_t mode);
    InputMode_t getMode(void);
    bool isShiftActive(void);
    
    // Touches spéciales
    bool isCallPressed(void);
    bool isEndPressed(void);
    bool isUpPressed(void);
    bool isDownPressed(void);
    
    // Utilitaires
    const char* getModeString(void);
    void waitForRelease(void);
    bool isKeyPressed(char key);

private:
    struct KeyMapping {
        const char* lowercase;
        const char* uppercase;
        const char* numbers;
        const char* symbols;
        uint8_t count_lower;
        uint8_t count_upper;
        uint8_t count_num;
        uint8_t count_sym;
    };
    
    KeyMapping keyMap[10];
    
    uint32_t lastKeyTime;
    char lastKey;
    uint8_t tapCount;
    static const uint32_t TAP_TIMEOUT_MS = 800;
    
    InputMode_t currentMode;
    bool capsLock;
    
    void initKeyMap(uint8_t digit, const char* lower, const char* upper, 
                   const char* num, const char* sym);
    char getCharFromTap(char key, uint8_t tapIdx);
    void resetMultiTapState(void);
};

#endif // KEYBOARD_H