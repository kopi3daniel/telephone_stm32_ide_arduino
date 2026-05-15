

#include "keyboard.h"
#include <Keypad.h>

// Objet Keypad global
Keypad keypad = Keypad(
    makeKeymap((char*)KEYPAD_LAYOUT),
    (uint8_t*)KEYPAD_ROW_PINS,
    (uint8_t*)KEYPAD_COL_PINS,
    KEYPAD_ROWS,
    KEYPAD_COLS
);

Keyboard::Keyboard() {
    lastKey = 0;
    lastKeyTime = 0;
    tapCount = 0;
    currentMode = INPUT_MODE_LOWERCASE;
    capsLock = false;
    
    initKeyMap(0, " .,", " .,", "0", "?!'-");
    initKeyMap(1, ".,!?", ".,!?", "1", "@:/;");
    initKeyMap(2, "abc", "ABC", "2", "àâæ");
    initKeyMap(3, "def", "DEF", "3", "èéê");
    initKeyMap(4, "ghi", "GHI", "4", "îïì");
    initKeyMap(5, "jkl", "JKL", "5", "ĵķł");
    initKeyMap(6, "mno", "MNO", "6", "ôöò");
    initKeyMap(7, "pqrs", "PQRS", "7", "śšş");
    initKeyMap(8, "tuv", "TUV", "8", "ùúû");
    initKeyMap(9, "wxyz", "WXYZ", "9", "ýÿż");
}

void Keyboard::initKeyMap(uint8_t digit, const char* lower, const char* upper,
                         const char* num, const char* sym) {
    if (digit > 9) return;
    keyMap[digit].lowercase = lower;
    keyMap[digit].uppercase = upper;
    keyMap[digit].numbers   = num;
    keyMap[digit].symbols   = sym;
    keyMap[digit].count_lower = strlen(lower);
    keyMap[digit].count_upper = strlen(upper);
    keyMap[digit].count_num   = strlen(num);
    keyMap[digit].count_sym   = strlen(sym);
}

void Keyboard::init(void) {
    pinMode(BTN_CALL_PIN, INPUT_PULLUP);
    pinMode(BTN_END_PIN, INPUT_PULLUP);
}

char Keyboard::scanKey(void) {
    char key = keypad.getKey();
    if (key != NO_KEY) {
        while (keypad.getState() == PRESSED || keypad.getState() == HOLD) {
            delay(10);
        }
        return key;
    }
    return 0;
}

char Keyboard::multiTapInput(char key) {
    uint32_t now = millis();
    
    if (key == 'J') {
        if (lastKey != 0) confirmMultiTap();
        toggleMode();
        return 0;
    }
    
    if (key == 'I') {
        if (lastKey != 0) confirmMultiTap();
        toggleShift();
        return capsLock ? '^' : 'v';
    }
    
    if (key >= '0' && key <= '9') {
        if (key == lastKey) {
            if ((now - lastKeyTime) < TAP_TIMEOUT_MS) {
                tapCount++;
            } else {
                confirmMultiTap();
                tapCount = 0;
            }
        } else {
            if (lastKey != 0) confirmMultiTap();
            tapCount = 0;
        }
        
        lastKey = key;
        lastKeyTime = now;
        return getCharFromTap(key, tapCount);
    }
    
    if (lastKey != 0) confirmMultiTap();
    return key;
}

char Keyboard::confirmMultiTap(void) {
    if (lastKey == 0) return 0;
    char confirmed = getCharFromTap(lastKey, tapCount);
    resetMultiTapState();
    return confirmed;
}

void Keyboard::checkMultiTapTimeout(void) {
    if (lastKey != 0) {
        if ((millis() - lastKeyTime) >= TAP_TIMEOUT_MS) {
            confirmMultiTap();
        }
    }
}

char Keyboard::getCharFromTap(char key, uint8_t tapIdx) {
    if (key < '0' || key > '9') return key;
    
    uint8_t digit = key - '0';
    const char* chars = NULL;
    uint8_t count = 0;
    
    if (currentMode == INPUT_MODE_UPPERCASE || 
        (currentMode == INPUT_MODE_LOWERCASE && capsLock)) {
        chars = keyMap[digit].uppercase;
        count = keyMap[digit].count_upper;
    } else if (currentMode == INPUT_MODE_LOWERCASE) {
        chars = keyMap[digit].lowercase;
        count = keyMap[digit].count_lower;
    } else if (currentMode == INPUT_MODE_NUMBERS) {
        chars = keyMap[digit].numbers;
        count = keyMap[digit].count_num;
    } else if (currentMode == INPUT_MODE_SYMBOLS) {
        chars = keyMap[digit].symbols;
        count = keyMap[digit].count_sym;
    }
    
    if (chars == NULL || count == 0) return key;
    return chars[tapIdx % count];
}

void Keyboard::resetMultiTapState(void) {
    lastKey = 0;
    tapCount = 0;
    lastKeyTime = 0;
}

void Keyboard::toggleMode(void) {
    switch (currentMode) {
        case INPUT_MODE_LOWERCASE: currentMode = INPUT_MODE_UPPERCASE; break;
        case INPUT_MODE_UPPERCASE: currentMode = INPUT_MODE_NUMBERS;   break;
        case INPUT_MODE_NUMBERS:   currentMode = INPUT_MODE_SYMBOLS;   break;
        case INPUT_MODE_SYMBOLS:   currentMode = INPUT_MODE_LOWERCASE; break;
        default:                   currentMode = INPUT_MODE_LOWERCASE; break;
    }
    capsLock = false;
    resetMultiTapState();
}

void Keyboard::toggleShift(void) {
    capsLock = !capsLock;
    resetMultiTapState();
}

void Keyboard::setMode(InputMode_t mode) {
    if (mode >= INPUT_MODE_LOWERCASE && mode <= INPUT_MODE_SYMBOLS) {
        currentMode = mode;
    }
    capsLock = false;
    resetMultiTapState();
}

InputMode_t Keyboard::getMode(void) {
    return currentMode;
}

bool Keyboard::isShiftActive(void) {
    return capsLock;
}

bool Keyboard::isCallPressed(void) {
    if (digitalRead(BTN_CALL_PIN) == LOW) {
        delay(20);
        if (digitalRead(BTN_CALL_PIN) == LOW) {
            while (digitalRead(BTN_CALL_PIN) == LOW) delay(10);
            return true;
        }
    }
    return false;
}

bool Keyboard::isEndPressed(void) {
    if (digitalRead(BTN_END_PIN) == LOW) {
        delay(20);
        if (digitalRead(BTN_END_PIN) == LOW) {
            while (digitalRead(BTN_END_PIN) == LOW) delay(10);
            return true;
        }
    }
    return false;
}

bool Keyboard::isUpPressed(void) {
    char key = scanKey();
    return (key == '2');
}

bool Keyboard::isDownPressed(void) {
    char key = scanKey();
    return (key == '8');
}

const char* Keyboard::getModeString(void) {
    static const char* modeStrings[] = {"abc", "ABC", "123", "!@#"};
    if (currentMode <= INPUT_MODE_SYMBOLS) {
        return modeStrings[currentMode];
    }
    return "???";
}

void Keyboard::waitForRelease(void) {
    while (keypad.getKey() != NO_KEY) delay(10);
    delay(50);
}

bool Keyboard::isKeyPressed(char key) {
    return (scanKey() == key);
}