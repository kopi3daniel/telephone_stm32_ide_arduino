/*


  ORDRE IMPORTANT : audio.h en premier (il inclut Arduino.h et config.h)

 */
#include "audio.h"

// ============================================
// TABLES IMA ADPCM (STANDARD)
// ============================================

// Table des pas de quantification (89 valeurs)
const int16_t Audio::stepSizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

// Table d'ajustement d'index (16 valeurs)
const int8_t Audio::indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

// ============================================
// CONSTRUCTEUR
// ============================================
Audio::Audio() {
    microphoneEnabled = false;
    speakerEnabled = false;
    muted = false;
    volume = 50;
    isSpeaking = false;
    vadThreshold = VAD_THRESHOLD;
    lastVoiceTime = 0;
    silenceCounter = 0;
    bufferIndex = 0;
    bufferReady = false;
    captureBuffer = bufferA;
    playbackBuffer = bufferB;
    lastSampleTime = 0;
    sampleIntervalUs = 1000000 / AUDIO_SAMPLE_RATE;
    
    resetADPCMEncoder();
    resetADPCMDecoder();
}

// ============================================
// INITIALISATION
// ============================================
void Audio::init(void) {
    pinMode(AUDIO_MIC_PIN, INPUT);
    pinMode(AUDIO_SPK_PIN, OUTPUT);
    
    enableMicrophone(true);
    enableSpeaker(true);
}

// ============================================
// RÉINITIALISATION ADPCM
// ============================================
void Audio::resetADPCMEncoder(void) {
    adpcm_predSample = 0;
    adpcm_index = 0;
}

void Audio::resetADPCMDecoder(void) {
    adpcm_predSampleDec = 0;
    adpcm_indexDec = 0;
}

// ============================================
// CONTRÔLE DU MICROPHONE
// ============================================
void Audio::enableMicrophone(bool enable) {
    microphoneEnabled = enable;
}

void Audio::muteMicrophone(bool mute) {
    muted = mute;
}

bool Audio::isMicrophoneEnabled(void) {
    return microphoneEnabled;
}

// ============================================
// CONTRÔLE DU HAUT-PARLEUR
// ============================================
void Audio::enableSpeaker(bool enable) {
    speakerEnabled = enable;
}

void Audio::setVolume(uint8_t vol) {
    if (vol > 100) vol = 100;
    volume = vol;
}

uint8_t Audio::getVolume(void) {
    return volume;
}

bool Audio::isSpeakerEnabled(void) {
    return speakerEnabled;
}

// ============================================
// LECTURE MICROPHONE
// ============================================
uint8_t Audio::readMicSample(void) {
    if (!microphoneEnabled) return 128;
    
    int adcValue = analogRead(AUDIO_MIC_PIN);
    uint8_t sample = (uint8_t)(adcValue >> 2);
    
    detectVoice(sample);
    return sample;
}

// ============================================
// LECTURE HAUT-PARLEUR
// ============================================
void Audio::playSpeakerSample(uint8_t sample) {
    if (!speakerEnabled) return;
    
    uint8_t adjustedSample;
    if (muted) {
        adjustedSample = 0;
    } else {
        adjustedSample = (uint8_t)((uint16_t)sample * volume / 100);
    }
    
    analogWrite(AUDIO_SPK_PIN, adjustedSample);
}

// ============================================
// DÉTECTION DE VOIX (VAD)
// ============================================
bool Audio::detectVoice(uint8_t sample) {
    // Calculer l'amplitude par rapport au centre (128)
    int16_t amplitude = abs((int16_t)sample - 128);
    
    if (amplitude > vadThreshold) {
        lastVoiceTime = millis();
        isSpeaking = true;
        silenceCounter = 0;
        return true;
    }
    
    return false;
}

bool Audio::isVoiceDetected(void) {
    return isSpeaking;
}

bool Audio::isSilence(void) {
    // Vérifier si le silence a duré plus que SILENCE_TIMEOUT_MS
    if (isSpeaking && (millis() - lastVoiceTime > SILENCE_TIMEOUT_MS)) {
        isSpeaking = false;
        return true;
    }
    return !isSpeaking;
}

void Audio::setVADThreshold(uint16_t threshold) {
    vadThreshold = threshold;
}

// ============================================
// ÉTAT
// ============================================
bool Audio::isMuted(void) {
    return muted;
}

bool Audio::isSpeakingNow(void) {
    return isSpeaking;
}

void Audio::reset(void) {
    bufferIndex = 0;
    bufferReady = false;
    captureBuffer = bufferA;
    playbackBuffer = bufferB;
    resetADPCMEncoder();
    resetADPCMDecoder();
}

// ============================================
// CAPTURE AUDIO
// ============================================
uint16_t Audio::captureAudio(uint8_t* buffer, uint16_t maxLen) {
    if (!microphoneEnabled) return 0;
    
    uint16_t samplesRead = 0;
    
    for (uint16_t i = 0; i < maxLen; i++) {
        // Respecter la fréquence d'échantillonnage
        uint32_t now = micros();
        if (now - lastSampleTime < sampleIntervalUs) {
            delayMicroseconds(sampleIntervalUs - (now - lastSampleTime));
        }
        lastSampleTime = micros();
        
        buffer[i] = readMicSample();
        samplesRead++;
    }
    
    return samplesRead;
}

bool Audio::isBufferReady(void) {
    return bufferReady;
}

AudioPacket_t Audio::getCaptureBuffer(void) {
    AudioPacket_t packet;
    memcpy(packet.data, captureBuffer, AUDIO_BUFFER_SIZE);
    packet.length = AUDIO_BUFFER_SIZE;
    packet.compressed = false;
    bufferReady = false;
    bufferIndex = 0;
    return packet;
}

// ============================================
// LECTURE AUDIO
// ============================================
void Audio::playAudio(uint8_t* buffer, uint16_t length) {
    if (!speakerEnabled) return;
    
    for (uint16_t i = 0; i < length; i++) {
        playSpeakerSample(buffer[i]);
        delayMicroseconds(sampleIntervalUs);
    }
}

void Audio::playPacket(AudioPacket_t* packet) {
    if (packet == NULL) return;
    
    if (packet->compressed && ADPCM_ENABLED) {
        uint8_t decompressed[AUDIO_BUFFER_SIZE];
        uint16_t decodedLen = adpcmDecode(packet->data, decompressed, packet->length);
        playAudio(decompressed, decodedLen);
    } else {
        playAudio(packet->data, packet->length);
    }
}

// ============================================
// COMPRESSION ADPCM - ENCODEUR
// ============================================
uint16_t Audio::adpcmEncode(uint8_t* input, uint8_t* output, uint16_t length) {
    
    if (length % 2 != 0) {
        length--;
    }
    
    for (uint16_t i = 0; i < length; i += 2) {
        
        int16_t sample1 = ((int16_t)input[i] - 128) << 8;
        int32_t diff1 = sample1 - adpcm_predSample;
        
        int16_t step = stepSizeTable[adpcm_index];
        uint8_t code1 = 0;
        
        if (diff1 < 0) {
            code1 = 8;
            diff1 = -diff1;
        }
        
        if (diff1 >= step) { code1 |= 4; diff1 -= step; }
        step >>= 1;
        if (diff1 >= step) { code1 |= 2; diff1 -= step; }
        step >>= 1;
        if (diff1 >= step) { code1 |= 1; }
        
        step = stepSizeTable[adpcm_index];
        int32_t diffq1 = 0;
        if (code1 & 4) diffq1 += step;
        if (code1 & 2) diffq1 += step >> 1;
        if (code1 & 1) diffq1 += step >> 2;
        diffq1 += step >> 3;
        
        if (code1 & 8) {
            adpcm_predSample -= diffq1;
        } else {
            adpcm_predSample += diffq1;
        }
        
        if (adpcm_predSample > 32767) adpcm_predSample = 32767;
        if (adpcm_predSample < -32768) adpcm_predSample = -32768;
        
        adpcm_index += indexTable[code1];
        if (adpcm_index < 0)  adpcm_index = 0;
        if (adpcm_index > 88) adpcm_index = 88;
        
        // Échantillon 2
        int16_t sample2 = ((int16_t)input[i + 1] - 128) << 8;
        int32_t diff2 = sample2 - adpcm_predSample;
        
        step = stepSizeTable[adpcm_index];
        uint8_t code2 = 0;
        
        if (diff2 < 0) {
            code2 = 8;
            diff2 = -diff2;
        }
        
        if (diff2 >= step) { code2 |= 4; diff2 -= step; }
        step >>= 1;
        if (diff2 >= step) { code2 |= 2; diff2 -= step; }
        step >>= 1;
        if (diff2 >= step) { code2 |= 1; }
        
        step = stepSizeTable[adpcm_index];
        int32_t diffq2 = 0;
        if (code2 & 4) diffq2 += step;
        if (code2 & 2) diffq2 += step >> 1;
        if (code2 & 1) diffq2 += step >> 2;
        diffq2 += step >> 3;
        
        if (code2 & 8) {
            adpcm_predSample -= diffq2;
        } else {
            adpcm_predSample += diffq2;
        }
        
        if (adpcm_predSample > 32767) adpcm_predSample = 32767;
        if (adpcm_predSample < -32768) adpcm_predSample = -32768;
        
        adpcm_index += indexTable[code2];
        if (adpcm_index < 0)  adpcm_index = 0;
        if (adpcm_index > 88) adpcm_index = 88;
        
        output[i / 2] = (code1 << 4) | (code2 & 0x0F);
    }
    
    return length / 2;
}

// ============================================
// DÉCOMPRESSION ADPCM - DÉCODEUR
// ============================================
uint16_t Audio::adpcmDecode(uint8_t* input, uint8_t* output, uint16_t length) {
    
    if (length == 0) return 0;
    
    for (uint16_t i = 0; i < length; i++) {
        
        uint8_t code1 = (input[i] >> 4) & 0x0F;
        uint8_t code2 = input[i] & 0x0F;
        
        // Décoder échantillon 1
        int16_t step = stepSizeTable[adpcm_indexDec];
        int32_t diff1 = 0;
        if (code1 & 4) diff1 += step;
        if (code1 & 2) diff1 += step >> 1;
        if (code1 & 1) diff1 += step >> 2;
        diff1 += step >> 3;
        
        if (code1 & 8) {
            adpcm_predSampleDec -= diff1;
        } else {
            adpcm_predSampleDec += diff1;
        }
        
        if (adpcm_predSampleDec > 32767) adpcm_predSampleDec = 32767;
        if (adpcm_predSampleDec < -32768) adpcm_predSampleDec = -32768;
        
        int16_t sample1 = (adpcm_predSampleDec >> 8) + 128;
        if (sample1 > 255) sample1 = 255;
        if (sample1 < 0)   sample1 = 0;
        output[i * 2] = (uint8_t)sample1;
        
        adpcm_indexDec += indexTable[code1];
        if (adpcm_indexDec < 0)  adpcm_indexDec = 0;
        if (adpcm_indexDec > 88) adpcm_indexDec = 88;
        
        // Décoder échantillon 2
        step = stepSizeTable[adpcm_indexDec];
        int32_t diff2 = 0;
        if (code2 & 4) diff2 += step;
        if (code2 & 2) diff2 += step >> 1;
        if (code2 & 1) diff2 += step >> 2;
        diff2 += step >> 3;
        
        if (code2 & 8) {
            adpcm_predSampleDec -= diff2;
        } else {
            adpcm_predSampleDec += diff2;
        }
        
        if (adpcm_predSampleDec > 32767) adpcm_predSampleDec = 32767;
        if (adpcm_predSampleDec < -32768) adpcm_predSampleDec = -32768;
        
        int16_t sample2 = (adpcm_predSampleDec >> 8) + 128;
        if (sample2 > 255) sample2 = 255;
        if (sample2 < 0)   sample2 = 0;
        output[i * 2 + 1] = (uint8_t)sample2;
        
        adpcm_indexDec += indexTable[code2];
        if (adpcm_indexDec < 0)  adpcm_indexDec = 0;
        if (adpcm_indexDec > 88) adpcm_indexDec = 88;
    }
    
    return length * 2;
}

// ============================================
// TONALITÉS ET EFFETS SONORES
// ============================================

// Joue une tonalité simple
void Audio::playTone(uint16_t frequency, uint16_t durationMs) {
    if (frequency == 0 || durationMs == 0) return;
    if (!speakerEnabled) return;
    
    bool wasMuted = muted;
    if (wasMuted) muted = false;
    
    tone(AUDIO_SPK_PIN, frequency, durationMs);
    delay(durationMs);
    noTone(AUDIO_SPK_PIN);
    
    if (wasMuted) muted = true;
}

// Joue une tonalité avec contrôle PWM
void Audio::playTonePWM(uint16_t frequency, uint16_t durationMs) {
    if (frequency == 0 || durationMs == 0) return;
    if (!speakerEnabled) return;
    
    uint32_t periodUs = 1000000 / frequency;
    uint32_t halfPeriodUs = periodUs / 2;
    uint8_t pwmValue = (uint8_t)((uint32_t)volume * 255 / 100);
    if (muted) pwmValue = 0;
    
    uint32_t cycles = (uint32_t)frequency * durationMs / 1000;
    
    for (uint32_t i = 0; i < cycles; i++) {
        analogWrite(AUDIO_SPK_PIN, pwmValue);
        delayMicroseconds(halfPeriodUs);
        analogWrite(AUDIO_SPK_PIN, 0);
        delayMicroseconds(halfPeriodUs);
    }
    
    analogWrite(AUDIO_SPK_PIN, 0);
}

// Sonnerie d'appel
void Audio::playRingTone(void) {
    for (int cycle = 0; cycle < 3; cycle++) {
        playTone(2000, 200);
        delay(100);
        playTone(2000, 200);
        delay(400);
    }
}

// Tonalité d'attente
void Audio::playDialTone(void) {
    for (int i = 0; i < 10; i++) {
        playTone(440, 250);
        delay(250);
    }
}

// Bip de notification
void Audio::playNotificationBeep(void) {
    playTone(1500, 100);
    delay(50);
    playTone(2000, 100);
}

// Arrêter toute tonalité
void Audio::stopTone(void) {
    noTone(AUDIO_SPK_PIN);
    analogWrite(AUDIO_SPK_PIN, 0);
}

// ============================================
// FIN DU FICHIER audio.cpp
// ============================================