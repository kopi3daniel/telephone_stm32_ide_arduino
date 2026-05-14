/**
 * config.h - Configuration globale du Téléphone LoRa
 * (Version Arduino IDE - Compatible STM32F103C8T6 et Blues)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

// ============================================
// 1. IDENTITÉ DU TÉLÉPHONE
// ============================================
#define PHONE_ID                    0x0001
#define PHONE_NUMBER                "0001"
#define PHONE_NAME                  "LoRaPhone_V1"

// ============================================
// 2. BROCHAGE (Format avec underscore)
// ============================================

// --- Module LoRa RA-02 (SPI1) ---
#define LORA_SPI                    SPI
#define LORA_NSS_PIN                PA_4
#define LORA_SCK_PIN                PA_5
#define LORA_MISO_PIN               PA_6
#define LORA_MOSI_PIN               PA_7
#define LORA_DIO0_PIN               PA_0
#define LORA_RST_PIN                PA_2

// --- Écran OLED 1.3" (I2C1) ---
#define OLED_I2C                    Wire
#define OLED_SDA_PIN                PB_7
#define OLED_SCL_PIN                PB_6
#define OLED_ADDRESS                0x3C
#define OLED_WIDTH                  128
#define OLED_HEIGHT                 64

// --- Clavier 4x6 (10 pins GPIO) ---
#define KEYPAD_ROWS                 6
#define KEYPAD_COLS                 4

// Lignes (Output)
const uint8_t KEYPAD_ROW_PINS[KEYPAD_ROWS] = {
    PB_12, PB_13, PB_14, PB_15, PA_8, PA_9
};

// Colonnes (Input)
const uint8_t KEYPAD_COL_PINS[KEYPAD_COLS] = {
    PB_3, PB_4, PB_5, PA_10
};

// Layout du clavier
const char KEYPAD_LAYOUT[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
    {'E', 'F', 'G', 'H'},
    {'I', 'J', 'K', 'L'}
};

// --- Audio ---
#define AUDIO_MIC_PIN               PA_1
#define AUDIO_SPK_PIN               PA_8

// --- LED Torche (PWM) ---
#define TORCH_PIN                   PA_11

// --- Boutons physiques ---
#define BTN_CALL_PIN                PB_8
#define BTN_END_PIN                 PB_9

// --- Buzzer et Vibreur ---
#define BUZZER_PIN                  PC_15
#define VIBRATOR_PIN                PC_14

// --- Mesure batterie ---
#define BATTERY_ADC_PIN             PA_0

// --- Debug UART ---
#define DEBUG_SERIAL                Serial1
#define DEBUG_BAUDRATE              115200

// ============================================
// 3. PARAMÈTRES LORA
// ============================================
#define LORA_FREQ                   868.0f

#define LORA_VOICE_BANDWIDTH        250.0f
#define LORA_VOICE_SPREADING_FACTOR 7
#define LORA_VOICE_CODING_RATE      5
#define LORA_VOICE_TX_POWER         17
#define LORA_VOICE_PREAMBLE_LENGTH  8
#define LORA_VOICE_CRC_ENABLED      false

#define LORA_DATA_BANDWIDTH         125.0f
#define LORA_DATA_SPREADING_FACTOR  9
#define LORA_DATA_CODING_RATE       8
#define LORA_DATA_TX_POWER          17
#define LORA_DATA_PREAMBLE_LENGTH   12
#define LORA_DATA_CRC_ENABLED       true

// ============================================
// 4. PARAMÈTRES AUDIO
// ============================================
#define AUDIO_SAMPLE_RATE           8000
#define AUDIO_BITS_PER_SAMPLE       8
#define AUDIO_BUFFER_SIZE           64
#define AUDIO_PACKET_TIME_MS        8

#define ADPCM_ENABLED               true
#define ADPCM_INPUT_SIZE            64
#define ADPCM_OUTPUT_SIZE           32

#define VAD_THRESHOLD               500
#define VAD_HOLD_TIME_MS            100
#define SILENCE_TIMEOUT_MS          50

// ============================================
// 5. PARAMÈTRES ÉNERGIE
// ============================================
#define BATTERY_FULL_MV             4200
#define BATTERY_75_MV               3900
#define BATTERY_50_MV               3700
#define BATTERY_25_MV               3500
#define BATTERY_LOW_MV              3400
#define BATTERY_CRITICAL_MV         3200

#define TIMEOUT_SCREEN_OFF_MS       30000
#define TIMEOUT_SLEEP_MS            300000
#define TIMEOUT_STOP_MS             3600000

#define POWER_ACTIVE_MA             120
#define POWER_SLEEP_MA              15
#define POWER_STOP_MA               2
#define POWER_STANDBY_UA            5

// ============================================
// 6. PROTOCOLES
// ============================================
#define PKT_SYNC_BYTE               0xAA
#define PKT_CALL_REQUEST            0x10
#define PKT_CALL_RINGING            0x11
#define PKT_CALL_ACCEPT             0x12
#define PKT_CALL_REJECT             0x13
#define PKT_CALL_END                0x14
#define PKT_CALL_BUSY               0x15
#define PKT_VOICE_DATA              0x20
#define PKT_SMS_TEXT                0x30
#define PKT_SMS_ACK                 0x31
#define PKT_SMS_DELIVERY            0x32
#define PKT_PING                    0x40
#define PKT_PONG                    0x41
#define PKT_PRESENCE                0x50
#define PKT_CONTACT_REQUEST         0x60
#define PKT_CONTACT_SEND            0x61

#define PKT_HEADER_SIZE             7
#define PKT_MAX_PAYLOAD             250

#define SMS_MAX_LENGTH              160
#define SMS_MAX_CONVERSATIONS       20
#define SMS_MAX_PER_CONVERSATION    50

// ============================================
// 7. PARAMÈTRES INTERFACE
// ============================================
#define SCREEN_WIDTH                128
#define SCREEN_HEIGHT               64
#define STATUS_BAR_Y                0
#define STATUS_BAR_HEIGHT           8
#define TITLE_BAR_Y                 10
#define TITLE_BAR_HEIGHT            10
#define CONTENT_START_Y             20
#define CONTENT_END_Y               52
#define BOTTOM_BAR_Y                54
#define BOTTOM_BAR_HEIGHT           10

#define FONT_SMALL                  1
#define FONT_NORMAL                 2

// ============================================
// 8. SÉCURITÉ
// ============================================
#define AES_ENABLED                 false
#define AES_KEY_SIZE                16

const uint8_t AES_DEFAULT_KEY[AES_KEY_SIZE] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

#define DEFAULT_PIN                 "0000"
#define PIN_LENGTH                  4

// ============================================
// 9. DÉBOGAGE
// ============================================
#define DEBUG_ENABLED               true

#if DEBUG_ENABLED
    #define LOG_INFO(msg, ...)      { char _buf[128]; snprintf(_buf, 128, "[INFO] " msg "\r\n", ##__VA_ARGS__); DEBUG_SERIAL.print(_buf); }
    #define LOG_WARN(msg, ...)      { char _buf[128]; snprintf(_buf, 128, "[WARN] " msg "\r\n", ##__VA_ARGS__); DEBUG_SERIAL.print(_buf); }
    #define LOG_ERROR(msg, ...)     { char _buf[128]; snprintf(_buf, 128, "[ERROR] " msg "\r\n", ##__VA_ARGS__); DEBUG_SERIAL.print(_buf); }
    #define LOG_DEBUG(msg, ...)     { char _buf[128]; snprintf(_buf, 128, "[DEBUG] " msg "\r\n", ##__VA_ARGS__); DEBUG_SERIAL.print(_buf); }
#else
    #define LOG_INFO(msg, ...)
    #define LOG_WARN(msg, ...)
    #define LOG_ERROR(msg, ...)
    #define LOG_DEBUG(msg, ...)
#endif

// ============================================
// 10. MACROS UTILES
// ============================================
#define ARRAY_SIZE(arr)             (sizeof(arr) / sizeof(arr[0]))
#define MIN(a, b)                   ((a) < (b) ? (a) : (b))
#define MAX(a, b)                   ((a) > (b) ? (a) : (b))
#define CLAMP(x, min, max)          (MIN(MAX(x, min), max))

#endif // CONFIG_H