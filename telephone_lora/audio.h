/**

 * Ce fichier declare la classe Audio qui gere :
 * - La capture du microphone via l ADC (analogRead)
 * - La lecture audio via PWM (analogWrite / tone)
 * - La compression/decompression ADPCM
 * - La detection de voix (VAD - Voice Activity Detection)
 * - Le double buffering pour une capture/lecture fluide
 * 
 * Brochage utilise (defini dans config.h) :
 * - AUDIO_MIC_PIN  (PA1) : Entree analogique microphone
 * - AUDIO_SPK_PIN  (PA8) : Sortie PWM haut-parleur
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "config.h"

// ============================================
// STRUCTURE POUR UN PAQUET AUDIO
// ============================================
typedef struct {
    uint8_t data[AUDIO_BUFFER_SIZE];
    uint16_t length;
    bool compressed;
} AudioPacket_t;

// ============================================
// CLASSE AUDIO
// ============================================
class Audio {
    
private:
    // Buffers audio (double buffering)
    uint8_t bufferA[AUDIO_BUFFER_SIZE];
    uint8_t bufferB[AUDIO_BUFFER_SIZE];
    uint8_t* captureBuffer;
    uint8_t* playbackBuffer;
    uint16_t bufferIndex;
    bool bufferReady;
    
    // Etat audio
    bool microphoneEnabled;
    bool speakerEnabled;
    bool muted;
    uint8_t volume;
    bool isSpeaking;
    
    // VAD (Voice Activity Detection)
    uint16_t vadThreshold;
    uint32_t lastVoiceTime;
    uint16_t silenceCounter;
    
    // ADPCM
    int16_t adpcm_predSample;
    int8_t  adpcm_index;
    int16_t adpcm_predSampleDec;
    int8_t  adpcm_indexDec;
    
    // Tables ADPCM
    static const int16_t stepSizeTable[89];
    static const int8_t indexTable[16];
    
    // Timing
    uint32_t lastSampleTime;
    uint32_t sampleIntervalUs;
    
    // Methodes privees
    uint8_t readMicSample(void);
    void playSpeakerSample(uint8_t sample);
    bool detectVoice(uint8_t sample);
    void resetADPCMEncoder(void);
    void resetADPCMDecoder(void);
    
public:
    // Constructeur et initialisation
    Audio();
    void init(void);
    
    // Controle du microphone
    void enableMicrophone(bool enable);
    void muteMicrophone(bool mute);
    bool isMicrophoneEnabled(void);
    
    // Controle du haut-parleur
    void enableSpeaker(bool enable);
    void setVolume(uint8_t vol);
    uint8_t getVolume(void);
    bool isSpeakerEnabled(void);
    
    // Capture audio
    uint16_t captureAudio(uint8_t* buffer, uint16_t maxLen);
    bool isBufferReady(void);
    AudioPacket_t getCaptureBuffer(void);
    
    // Lecture audio
    void playAudio(uint8_t* buffer, uint16_t length);
    void playPacket(AudioPacket_t* packet);
    
    // VAD
    bool isVoiceDetected(void);
    bool isSilence(void);
    void setVADThreshold(uint16_t threshold);
    
    // Compression ADPCM
    uint16_t adpcmEncode(uint8_t* input, uint8_t* output, uint16_t length);
    uint16_t adpcmDecode(uint8_t* input, uint8_t* output, uint16_t length);
    
    // Tonalites et effets sonores
    void playTone(uint16_t frequency, uint16_t durationMs);
    void playTonePWM(uint16_t frequency, uint16_t durationMs);
    void playRingTone(void);
    void playDialTone(void);
    void playNotificationBeep(void);
    void stopTone(void);
    
    // Etat et diagnostic
    bool isMuted(void);
    bool isSpeakingNow(void);
    void reset(void);
};

#endif // AUDIO_H