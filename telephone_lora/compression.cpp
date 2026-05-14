/**
 * ---------------------------------------------------------------------------
 * compression.cpp - Implementation ADPCM IMA
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "compression.h"
#include <string.h>

// ============================================
// TABLE DES PAS DE QUANTIFICATION IMA ADPCM
// ============================================
const int16_t Compression::stepSizeTable[89] = {
    7,     8,     9,     10,    11,    12,    13,    14,
    16,    17,    19,    21,    23,    25,    28,    31,
    34,    37,    41,    45,    50,    55,    60,    66,
    73,    80,    88,    97,    107,   118,   130,   143,
    157,   173,   190,   209,   230,   253,   279,   307,
    337,   371,   408,   449,   494,   544,   598,   658,
    724,   796,   876,   963,   1060,  1166,  1282,  1411,
    1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
    3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
    7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
    32767
};

// ============================================
// TABLE D AJUSTEMENT D INDEX
// ============================================
const int8_t Compression::indexAdjustTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

// ============================================
// CONSTRUCTEUR
// ============================================
Compression::Compression() {
    init();
}

void Compression::init(void) {
    resetAll();
}

// ============================================
// REINITIALISATION
// ============================================
void Compression::resetEncoder(void) {
    encoderState.predictedSample = 0;
    encoderState.stepIndex = 0;
}

void Compression::resetDecoder(void) {
    decoderState.predictedSample = 0;
    decoderState.stepIndex = 0;
}

void Compression::resetAll(void) {
    resetEncoder();
    resetDecoder();
}

// ============================================
// COMPRESSION ADPCM (ENCODE)
// ============================================
uint16_t Compression::encode(uint8_t* input, uint8_t* output, uint16_t length) {
    
    if (input == NULL || output == NULL || length == 0) return 0;
    
    uint16_t outputIndex = 0;
    
    for (uint16_t i = 0; i < length; i += 2) {
        
        int16_t sample1 = u8toS16(input[i]);
        uint8_t code1 = encodeSample(sample1);
        
        uint8_t code2 = 0;
        if (i + 1 < length) {
            int16_t sample2 = u8toS16(input[i + 1]);
            code2 = encodeSample(sample2);
        }
        
        output[outputIndex] = (code1 << 4) | (code2 & 0x0F);
        outputIndex++;
    }
    
    return outputIndex;
}

// ============================================
// ENCODAGE D UN SEUL ECHANTILLON
// ============================================
uint8_t Compression::encodeSample(int16_t sample) {
    
    int16_t step = stepSizeTable[encoderState.stepIndex];
    int32_t diff = sample - encoderState.predictedSample;
    
    uint8_t code = 0;
    
    if (diff < 0) {
        code = 8;
        diff = -diff;
    }
    
    if (diff >= step) { code |= 4; diff -= step; }
    step >>= 1;
    if (diff >= step) { code |= 2; diff -= step; }
    step >>= 1;
    if (diff >= step) { code |= 1; }
    
    step = stepSizeTable[encoderState.stepIndex];
    
    int32_t diffQ = 0;
    if (code & 4) diffQ += step;
    if (code & 2) diffQ += step >> 1;
    if (code & 1) diffQ += step >> 2;
    diffQ += step >> 3;
    
    if (code & 8) {
        encoderState.predictedSample -= diffQ;
    } else {
        encoderState.predictedSample += diffQ;
    }
    
    clampPredictedSample(&encoderState);
    
    encoderState.stepIndex += indexAdjustTable[code];
    clampIndex(&encoderState);
    
    return code;
}

// ============================================
// DECOMPRESSION ADPCM (DECODE)
// ============================================
uint16_t Compression::decode(uint8_t* input, uint8_t* output, uint16_t length) {
    
    if (input == NULL || output == NULL || length == 0) return 0;
    
    uint16_t outputIndex = 0;
    
    for (uint16_t i = 0; i < length; i++) {
        
        uint8_t code1 = (input[i] >> 4) & 0x0F;
        uint8_t code2 = input[i] & 0x0F;
        
        int16_t sample1 = decodeSample(code1);
        output[outputIndex] = s16toU8(sample1);
        outputIndex++;
        
        int16_t sample2 = decodeSample(code2);
        output[outputIndex] = s16toU8(sample2);
        outputIndex++;
    }
    
    return outputIndex;
}

// ============================================
// DECODAGE D UN SEUL CODE
// ============================================
int16_t Compression::decodeSample(uint8_t code) {
    
    int16_t step = stepSizeTable[decoderState.stepIndex];
    
    int32_t diff = 0;
    if (code & 4) diff += step;
    if (code & 2) diff += step >> 1;
    if (code & 1) diff += step >> 2;
    diff += step >> 3;
    
    if (code & 8) {
        decoderState.predictedSample -= diff;
    } else {
        decoderState.predictedSample += diff;
    }
    
    clampPredictedSample(&decoderState);
    
    decoderState.stepIndex += indexAdjustTable[code];
    clampIndex(&decoderState);
    
    return decoderState.predictedSample;
}

// ============================================
// FONCTIONS DE LIMITATION
// ============================================
void Compression::clampIndex(ADPCMState_t* state) {
    if (state->stepIndex < 0)  state->stepIndex = 0;
    if (state->stepIndex > 88) state->stepIndex = 88;
}

void Compression::clampPredictedSample(ADPCMState_t* state) {
    if (state->predictedSample > 32767) state->predictedSample = 32767;
    if (state->predictedSample < -32768) state->predictedSample = -32768;
}

// ============================================
// CONVERSION DE FORMATS
// ============================================
int16_t Compression::u8toS16(uint8_t sample) {
    return ((int16_t)sample - 128) << 8;
}

uint8_t Compression::s16toU8(int16_t sample) {
    int16_t reduced = (sample >> 8) + 128;
    if (reduced > 255) reduced = 255;
    if (reduced < 0)   reduced = 0;
    return (uint8_t)reduced;
}

// ============================================
// INFORMATIONS
// ============================================
float Compression::getCompressionRatio(void) { return 2.0f; }
uint16_t Compression::getCompressedSize(uint16_t originalSize) { return (originalSize + 1) / 2; }
uint16_t Compression::getDecompressedSize(uint16_t compressedSize) { return compressedSize * 2; }
bool Compression::isReady(void) { return true; }

// ============================================
// FIN DU FICHIER compression.cpp
// ============================================