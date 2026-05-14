/**
 * ---------------------------------------------------------------------------
 * compression.h - Compression/Decompression audio ADPCM pour Telephone LoRa
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Ce module implemente l algorithme ADPCM (Adaptive Differential PCM)
 * selon le standard IMA (Interactive Multimedia Association).
 * 
 * Caracteristiques :
 * - Compression 2:1 (8 bits -> 4 bits par echantillon)
 * - Qualite audio correcte pour la voix
 * - Faible latence (quelques millisecondes)
 * - Predicteur adaptatif avec table de pas
 * - Utilise pour compresser la voix avant transmission LoRa
 */

#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <Arduino.h>
#include "config.h"

// ============================================
// CONFIGURATION ADPCM
// ============================================
#define ADPCM_INPUT_SIZE        64
#define ADPCM_OUTPUT_SIZE       32
#define ADPCM_SAMPLES_PER_BYTE  2

// ============================================
// ETAT DU CODEUR/DECODEUR
// ============================================
typedef struct {
    int16_t predictedSample;
    int8_t  stepIndex;
} ADPCMState_t;

// ============================================
// CLASSE DE COMPRESSION
// ============================================
class Compression {
    
private:
    // ============================================
    // ETATS ADPCM
    // ============================================
    ADPCMState_t encoderState;
    ADPCMState_t decoderState;
    
    // ============================================
    // TABLES DE L ALGORITHME IMA ADPCM
    // ============================================
    
    static const int16_t stepSizeTable[89];
    static const int8_t indexAdjustTable[16];
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    uint8_t encodeSample(int16_t sample);
    int16_t decodeSample(uint8_t code);
    void clampIndex(ADPCMState_t* state);
    void clampPredictedSample(ADPCMState_t* state);
    
public:
    // ============================================
    // CONSTRUCTEUR
    // ============================================
    Compression();
    
    // ============================================
    // INITIALISATION
    // ============================================
    void init(void);
    
    // ============================================
    // COMPRESSION ADPCM
    // ============================================
    
    uint16_t encode(uint8_t* input, uint8_t* output, uint16_t length);
    uint16_t decode(uint8_t* input, uint8_t* output, uint16_t length);
    
    // ============================================
    // CONTROLE DU CODEC
    // ============================================
    
    void resetEncoder(void);
    void resetDecoder(void);
    void resetAll(void);
    
    // ============================================
    // INFORMATIONS
    // ============================================
    
    float getCompressionRatio(void);
    uint16_t getCompressedSize(uint16_t originalSize);
    uint16_t getDecompressedSize(uint16_t compressedSize);
    bool isReady(void);
    
    // ============================================
    // FONCTIONS UTILITAIRES
    // ============================================
    
    static int16_t u8toS16(uint8_t sample);
    static uint8_t s16toU8(int16_t sample);
};

#endif // COMPRESSION_H