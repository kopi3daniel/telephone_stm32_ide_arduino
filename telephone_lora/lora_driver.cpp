/**
 * ---------------------------------------------------------------------------
 * lora_driver.cpp - Implementation du driver LoRa RA-02 (SX1278)
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

//#include "lora_driver.h"

#include "lora_driver.h"
#include <Arduino.h>
#include <SPI.h>

// ============================================
// REGISTRES DU SX1278
// ============================================
#define REG_FIFO                    0x00
#define REG_OP_MODE                 0x01
#define REG_FRF_MSB                 0x06
#define REG_FRF_MID                 0x07
#define REG_FRF_LSB                 0x08
#define REG_PA_CONFIG               0x09
#define REG_PA_RAMP                 0x0A
#define REG_OCP                     0x0B
#define REG_LNA                     0x0C
#define REG_FIFO_ADDR_PTR           0x0D
#define REG_FIFO_TX_BASE_ADDR       0x0E
#define REG_FIFO_RX_BASE_ADDR       0x0F
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS_MASK          0x11
#define REG_IRQ_FLAGS               0x12
#define REG_RX_NB_BYTES             0x13
#define REG_RX_HEADER_CNT_VALUE_MSB 0x14
#define REG_RX_HEADER_CNT_VALUE_LSB 0x15
#define REG_RX_PACKET_CNT_VALUE_MSB 0x16
#define REG_RX_PACKET_CNT_VALUE_LSB 0x17
#define REG_MODEM_STAT              0x18
#define REG_PKT_SNR_VALUE           0x19
#define REG_PKT_RSSI_VALUE          0x1A
#define REG_RSSI_VALUE              0x1B
#define REG_HOP_CHANNEL             0x1C
#define REG_MODEM_CONFIG_1          0x1D
#define REG_MODEM_CONFIG_2          0x1E
#define REG_SYMB_TIMEOUT_LSB        0x1F
#define REG_PREAMBLE_MSB            0x20
#define REG_PREAMBLE_LSB            0x21
#define REG_PAYLOAD_LENGTH          0x22
#define REG_MAX_PAYLOAD_LENGTH      0x23
#define REG_HOP_PERIOD              0x24
#define REG_FIFO_RX_BYTE_ADDR       0x25
#define REG_MODEM_CONFIG_3          0x26
#define REG_FEI_MSB                 0x28
#define REG_FEI_MID                 0x28
#define REG_FEI_LSB                 0x29
#define REG_RSSI_WIDEBAND           0x2C
#define REG_DETECT_OPTIMIZE         0x31
#define REG_INVERTIQ                0x33
#define REG_DETECTION_THRESHOLD     0x37
#define REG_SYNC_WORD               0x39
#define REG_FIFO_TX_BYTE_ADDR       0x3A
#define REG_INVERTIQ2               0x3B
#define REG_VERSION                 0x42

// Modes
#define MODE_LONG_RANGE_MODE        0x80
#define MODE_SLEEP                  0x00
#define MODE_STDBY                  0x01
#define MODE_FSTX                   0x02
#define MODE_TX                     0x03
#define MODE_FSRX                   0x04
#define MODE_RX_CONTINUOUS          0x05
#define MODE_RX_SINGLE              0x06
#define MODE_CAD                    0x07

// Flags d'interruption
#define IRQ_CAD_DETECTED_MASK       0x01
#define IRQ_FHSS_CHANGE_CHANNEL     0x02
#define IRQ_CAD_DONE_MASK           0x04
#define IRQ_TX_DONE_MASK            0x08
#define IRQ_VALID_HEADER            0x10
#define IRQ_PAYLOAD_CRC_ERROR_MASK  0x20
#define IRQ_RX_DONE_MASK            0x40
#define IRQ_RX_TIMEOUT_MASK         0x80
#define IRQ_CRC_ERROR_MASK          0x20

// ============================================
// CONSTRUCTEUR
// ============================================
LoRaDriver::LoRaDriver() {
    currentMode = LORA_MODE_SLEEP;
    currentProfile = LORA_PROFILE_VOICE;
    packetAvailable = false;
    transmitting = false;
    frequency = LORA_FREQ;
    bandwidth = LORA_VOICE_BANDWIDTH;
    spreadingFactor = LORA_VOICE_SPREADING_FACTOR;
    codingRate = LORA_VOICE_CODING_RATE;
    txPower = LORA_VOICE_TX_POWER;
    preambleLength = LORA_VOICE_PREAMBLE_LENGTH;
    crcEnabled = LORA_VOICE_CRC_ENABLED;
    rxBufferIndex = 0;
    expectedSeqNum = 0;
    cyclicOnTime = 100;
    cyclicOffTime = 900;
    lastCyclicWake = 0;
    memset(&stats, 0, sizeof(stats));
}

// ============================================
// INITIALISATION
// ============================================
void LoRaDriver::init(void) {
    initGPIO();
    
    // Initialiser le bus SPI
    SPI.setMOSI(LORA_MOSI_PIN);
    SPI.setMISO(LORA_MISO_PIN);
    SPI.setSCLK(LORA_SCK_PIN);
    SPI.begin();
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    
    resetModule();
    
    // Verifier que le module repond
    uint8_t version = readRegister(REG_VERSION);
    if (version != 0x12) {
        // Erreur : module non detecte
        while (1) {
            digitalWrite(TORCH_PIN, !digitalRead(TORCH_PIN));
            delay(100);
        }
    }
    
    // Configuration par defaut
    setProfile(LORA_PROFILE_VOICE);
    setMode(LORA_MODE_STANDBY);
}

// ============================================
// CONFIGURATION GPIO
// ============================================
void LoRaDriver::initGPIO(void) {
    pinMode(LORA_NSS_PIN, OUTPUT);
    digitalWrite(LORA_NSS_PIN, HIGH);
    
    pinMode(LORA_RST_PIN, OUTPUT);
    digitalWrite(LORA_RST_PIN, HIGH);
    
    pinMode(LORA_DIO0_PIN, INPUT);
}

// ============================================
// RESET DU MODULE
// ============================================
void LoRaDriver::resetModule(void) {
    digitalWrite(LORA_RST_PIN, LOW);
    delay(10);
    digitalWrite(LORA_RST_PIN, HIGH);
    delay(10);
}

// ============================================
// LECTURE D'UN REGISTRE
// ============================================
uint8_t LoRaDriver::readRegister(uint8_t addr) {
    digitalWrite(LORA_NSS_PIN, LOW);
    
    SPI.transfer(addr & 0x7F);  // Bit 7 = 0 pour lecture
    uint8_t value = SPI.transfer(0x00);
    
    digitalWrite(LORA_NSS_PIN, HIGH);
    return value;
}

// ============================================
// ECRITURE D'UN REGISTRE
// ============================================
void LoRaDriver::writeRegister(uint8_t addr, uint8_t value) {
    digitalWrite(LORA_NSS_PIN, LOW);
    
    SPI.transfer(addr | 0x80);  // Bit 7 = 1 pour ecriture
    SPI.transfer(value);
    
    digitalWrite(LORA_NSS_PIN, HIGH);
}

// ============================================
// LECTURE EN RAFALE (BURST READ)
// ============================================
void LoRaDriver::readBurst(uint8_t addr, uint8_t* buffer, uint8_t length) {
    digitalWrite(LORA_NSS_PIN, LOW);
    
    SPI.transfer(addr & 0x7F);
    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = SPI.transfer(0x00);
    }
    
    digitalWrite(LORA_NSS_PIN, HIGH);
}

// ============================================
// ECRITURE EN RAFALE (BURST WRITE)
// ============================================
void LoRaDriver::writeBurst(uint8_t addr, uint8_t* buffer, uint8_t length) {
    digitalWrite(LORA_NSS_PIN, LOW);
    
    SPI.transfer(addr | 0x80);
    for (uint8_t i = 0; i < length; i++) {
        SPI.transfer(buffer[i]);
    }
    
    digitalWrite(LORA_NSS_PIN, HIGH);
}

// ============================================
// CONFIGURATION DU MODE
// ============================================
void LoRaDriver::setMode(LoRaMode_t mode) {
    uint8_t opMode;
    
    switch (mode) {
        case LORA_MODE_SLEEP:
            opMode = MODE_LONG_RANGE_MODE | MODE_SLEEP;
            break;
        case LORA_MODE_STANDBY:
            opMode = MODE_LONG_RANGE_MODE | MODE_STDBY;
            break;
        case LORA_MODE_TX:
            opMode = MODE_LONG_RANGE_MODE | MODE_TX;
            break;
        case LORA_MODE_RX_CONTINUOUS:
            opMode = MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS;
            break;
        case LORA_MODE_RX_CYCLIC:
            opMode = MODE_LONG_RANGE_MODE | MODE_RX_SINGLE;
            break;
        case LORA_MODE_CAD:
            opMode = MODE_LONG_RANGE_MODE | MODE_CAD;
            break;
        default:
            opMode = MODE_LONG_RANGE_MODE | MODE_STDBY;
    }
    
    writeRegister(REG_OP_MODE, opMode);
    currentMode = mode;
}

// ============================================
// CONFIGURATION DE LA FREQUENCE
// ============================================
void LoRaDriver::setFrequency(float freq) {
    frequency = freq;
    
    // Calculer la valeur du registre de frequence
    // Frequence = F_XOSC * Frf / 2^19
    // Frf = (Frequence * 2^19) / F_XOSC
    // F_XOSC = 32 MHz
    uint64_t frf = ((uint64_t)freq * 524288) / 32;
    
    writeRegister(REG_FRF_MSB, (frf >> 16) & 0xFF);
    writeRegister(REG_FRF_MID, (frf >> 8) & 0xFF);
    writeRegister(REG_FRF_LSB, frf & 0xFF);
}

// ============================================
// CONFIGURATION DES PARAMETRES LORA
// ============================================
void LoRaDriver::setLoRaParameters(void) {
    // Config 1 : Bande passante + Coding Rate
    uint8_t config1 = 0;
    
    // Bande passante
    if (bandwidth <= 125.0) {
        config1 |= 0x00;  // 125 kHz
    } else if (bandwidth <= 250.0) {
        config1 |= 0x80;  // 250 kHz
    } else {
        config1 |= 0x90;  // 500 kHz
    }
    
    // Coding rate
    switch (codingRate) {
        case 5: config1 |= 0x02; break;  // 4/5
        case 6: config1 |= 0x04; break;  // 4/6
        case 7: config1 |= 0x06; break;  // 4/7
        case 8: config1 |= 0x08; break;  // 4/8
        default: config1 |= 0x02; break;
    }
    
    writeRegister(REG_MODEM_CONFIG_1, config1);
    
    // Config 2 : Spreading Factor + CRC
    uint8_t config2 = 0;
    
    // Spreading factor
    config2 |= ((spreadingFactor & 0x0F) << 4);
    
    // CRC
    if (crcEnabled) {
        config2 |= 0x04;
    }
    
    writeRegister(REG_MODEM_CONFIG_2, config2);
    
    // Symbole timeout (pour la reception)
    if (spreadingFactor >= 10) {
        writeRegister(REG_SYMB_TIMEOUT_LSB, 0x08);
    }
}

// ============================================
// CONFIGURATION DE LA PUISSANCE D'EMISSION
// ============================================
void LoRaDriver::setTxPower(int8_t power) {
    txPower = power;
    
    if (power > 20) power = 20;
    if (power < 2)  power = 2;
    
    // Configuration PA_BOOST
    writeRegister(REG_PA_CONFIG, 0x80 | (power - 2));
    
    // Desactiver la limitation de courant
    writeRegister(REG_OCP, 0x3F);
}

// ============================================
// VERIFIER SI UNE RECEPTION EST EN COURS
// ============================================
bool LoRaDriver::isRxInProgress(void) {
    uint8_t irqFlags = readRegister(REG_IRQ_FLAGS);
    return !(irqFlags & IRQ_RX_DONE_MASK) && !(irqFlags & IRQ_RX_TIMEOUT_MASK);
}

// ============================================
// VIDER LE FIFO DE RECEPTION
// ============================================
void LoRaDriver::clearRxFIFO(void) {
    writeRegister(REG_IRQ_FLAGS, IRQ_RX_DONE_MASK | IRQ_RX_TIMEOUT_MASK);
}

// ============================================
// CONFIGURATION DU PROFIL (VOIX ou DATA)
// ============================================
void LoRaDriver::setProfile(LoRaProfile_t profile) {
    currentProfile = profile;
    
    // Passer en mode sleep pour modifier la configuration
    setMode(LORA_MODE_SLEEP);
    
    if (profile == LORA_PROFILE_VOICE) {
        frequency = LORA_FREQ;
        bandwidth = LORA_VOICE_BANDWIDTH;
        spreadingFactor = LORA_VOICE_SPREADING_FACTOR;
        codingRate = LORA_VOICE_CODING_RATE;
        txPower = LORA_VOICE_TX_POWER;
        preambleLength = LORA_VOICE_PREAMBLE_LENGTH;
        crcEnabled = LORA_VOICE_CRC_ENABLED;
        
        writeRegister(REG_MODEM_CONFIG_3, 0x04);
    } else {
        frequency = LORA_FREQ;
        bandwidth = LORA_DATA_BANDWIDTH;
        spreadingFactor = LORA_DATA_SPREADING_FACTOR;
        codingRate = LORA_DATA_CODING_RATE;
        txPower = LORA_DATA_TX_POWER;
        preambleLength = LORA_DATA_PREAMBLE_LENGTH;
        crcEnabled = LORA_DATA_CRC_ENABLED;
        
        writeRegister(REG_MODEM_CONFIG_3, 0x0C);
    }
    
    setFrequency(frequency);
    setLoRaParameters();
    setTxPower(txPower);
    
    writeRegister(REG_PREAMBLE_MSB, (preambleLength >> 8) & 0xFF);
    writeRegister(REG_PREAMBLE_LSB, preambleLength & 0xFF);
    
    setMode(LORA_MODE_STANDBY);
}

LoRaProfile_t LoRaDriver::getProfile(void) {
    return currentProfile;
}

// ============================================
// ENVOI D'UN PAQUET STRUCTURE
// ============================================
bool LoRaDriver::sendPacket(LoRaPacket_t* packet) {
    if (packet == nullptr) return false;
    
    uint16_t totalSize = PKT_HEADER_SIZE + packet->payloadSize;
    
    if (totalSize > 256) {
        totalSize = 256;
        packet->payloadSize = 256 - PKT_HEADER_SIZE;
    }
    
    uint8_t txBuffer[256];
    txBuffer[0] = packet->syncByte;
    txBuffer[1] = packet->packetType;
    txBuffer[2] = (packet->senderId >> 8) & 0xFF;
    txBuffer[3] = packet->senderId & 0xFF;
    txBuffer[4] = (packet->targetId >> 8) & 0xFF;
    txBuffer[5] = packet->targetId & 0xFF;
    txBuffer[6] = packet->payloadSize;
    
    if (packet->payloadSize > 0) {
        memcpy(&txBuffer[7], packet->payload, packet->payloadSize);
    }
    
    return sendRaw(txBuffer, totalSize);
}

// ============================================
// ENVOI DE DONNEES BRUTES
// ============================================
bool LoRaDriver::sendRaw(uint8_t* data, uint16_t length) {
    if (length == 0 || length > 256) return false;
    if (transmitting) return false;
    
    setMode(LORA_MODE_STANDBY);
    
    writeRegister(REG_PAYLOAD_LENGTH, length);
    writeRegister(REG_FIFO_TX_BASE_ADDR, 0x00);
    writeRegister(REG_FIFO_ADDR_PTR, 0x00);
    
    writeBurst(REG_FIFO, data, length);
    
    writeRegister(REG_IRQ_FLAGS, 0xFF);
    
    setMode(LORA_MODE_TX);
    transmitting = true;
    
    uint32_t timeout = millis() + 5000;
    
    while (millis() < timeout) {
        uint8_t irqFlags = readRegister(REG_IRQ_FLAGS);
        
        if (irqFlags & IRQ_TX_DONE_MASK) {
            stats.packetsSent++;
            stats.totalBytesSent += length;
            transmitting = false;
            
            writeRegister(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
            setMode(LORA_MODE_STANDBY);
            return true;
        }
        
        delay(1);
    }
    
    transmitting = false;
    setMode(LORA_MODE_STANDBY);
    return false;
}

// ============================================
// ENVOI EN BROADCAST
// ============================================
bool LoRaDriver::sendBroadcast(uint8_t* data, uint16_t length) {
    LoRaPacket_t packet;
    packet.syncByte = PKT_SYNC_BYTE;
    packet.packetType = data[1];
    packet.senderId = PHONE_ID;
    packet.targetId = 0xFFFF;
    packet.sequenceNum = 0;
    packet.payloadSize = length - PKT_HEADER_SIZE;
    
    if (packet.payloadSize > 0) {
        memcpy(packet.payload, data + PKT_HEADER_SIZE, packet.payloadSize);
    }
    
    return sendPacket(&packet);
}

bool LoRaDriver::isTransmitting(void) {
    return transmitting;
}

// ============================================
// PASSER EN MODE RECEPTION CONTINUE
// ============================================
void LoRaDriver::startReceive(void) {
    writeRegister(REG_IRQ_FLAGS, 0xFF);
    writeRegister(REG_FIFO_RX_BASE_ADDR, 0x00);
    writeRegister(REG_FIFO_ADDR_PTR, 0x00);
    setMode(LORA_MODE_RX_CONTINUOUS);
}

// ============================================
// PASSER EN MODE RECEPTION CYCLIQUE
// ============================================
void LoRaDriver::startCyclicReceive(uint32_t onTime, uint32_t offTime) {
    cyclicOnTime = onTime;
    cyclicOffTime = offTime;
    lastCyclicWake = millis();
    startReceive();
}

// ============================================
// VERIFIER SI UN PAQUET EST DISPONIBLE
// ============================================
bool LoRaDriver::isPacketAvailable(void) {
    return packetAvailable;
}

// ============================================
// LIRE LE PAQUET RECU
// ============================================
bool LoRaDriver::receivePacket(LoRaPacket_t* packet) {
    if (!packetAvailable || packet == nullptr) return false;
    
    uint8_t rxLength = readRegister(REG_RX_NB_BYTES);
    
    if (rxLength < PKT_HEADER_SIZE) {
        packetAvailable = false;
        stats.crcErrors++;
        return false;
    }
    
    uint8_t rxBuffer[256];
    writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_CURRENT_ADDR));
    readBurst(REG_FIFO, rxBuffer, rxLength);
    
    packet->syncByte = rxBuffer[0];
    packet->packetType = rxBuffer[1];
    packet->senderId = (rxBuffer[2] << 8) | rxBuffer[3];
    packet->targetId = (rxBuffer[4] << 8) | rxBuffer[5];
    packet->payloadSize = rxBuffer[6];
    
    if (packet->syncByte != PKT_SYNC_BYTE) {
        packetAvailable = false;
        stats.crcErrors++;
        return false;
    }
    
    if (packet->targetId != PHONE_ID && packet->targetId != 0xFFFF) {
        packetAvailable = false;
        return false;
    }
    
    if (packet->payloadSize > 0 && packet->payloadSize <= PKT_MAX_PAYLOAD) {
        memcpy(packet->payload, &rxBuffer[7], packet->payloadSize);
    }
    
    stats.lastRSSI = getRSSI();
    stats.lastSNR = getSNR();
    
    stats.packetsReceived++;
    stats.totalBytesReceived += rxLength;
    
    if (packet->sequenceNum > expectedSeqNum) {
        stats.packetsLost += (packet->sequenceNum - expectedSeqNum);
    }
    expectedSeqNum = packet->sequenceNum + 1;
    
    packetAvailable = false;
    writeRegister(REG_IRQ_FLAGS, IRQ_RX_DONE_MASK);
    
    return true;
}

// ============================================
// LECTURE DE DONNEES BRUTES
// ============================================
uint16_t LoRaDriver::receiveRaw(uint8_t* buffer, uint16_t maxLen) {
    if (!packetAvailable) return 0;
    
    uint8_t rxLength = readRegister(REG_RX_NB_BYTES);
    
    if (rxLength > maxLen) {
        rxLength = maxLen;
    }
    
    writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_CURRENT_ADDR));
    readBurst(REG_FIFO, buffer, rxLength);
    
    packetAvailable = false;
    writeRegister(REG_IRQ_FLAGS, IRQ_RX_DONE_MASK);
    
    return rxLength;
}

// ============================================
// GESTION DE L'INTERRUPTION DIO0
// ============================================
void LoRaDriver::handleInterrupt(void) {
    uint8_t irqFlags = readRegister(REG_IRQ_FLAGS);
    
    if (irqFlags & IRQ_RX_DONE_MASK) {
        if (irqFlags & IRQ_CRC_ERROR_MASK) {
            stats.crcErrors++;
            writeRegister(REG_IRQ_FLAGS, IRQ_CRC_ERROR_MASK);
        } else {
            packetAvailable = true;
        }
        writeRegister(REG_IRQ_FLAGS, IRQ_RX_DONE_MASK);
    }
    
    if (irqFlags & IRQ_TX_DONE_MASK) {
        transmitting = false;
        writeRegister(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
    }
}

// ============================================
// MESURE DU RSSI
// ============================================
int16_t LoRaDriver::getRSSI(void) {
    int16_t rssi = readRegister(REG_PKT_RSSI_VALUE);
    
    if (frequency > 800.0) {
        return rssi - 157;
    } else {
        return rssi - 164;
    }
}

// ============================================
// MESURE DU SNR
// ============================================
int8_t LoRaDriver::getSNR(void) {
    uint8_t snrValue = readRegister(REG_PKT_SNR_VALUE);
    return (int8_t)(snrValue) / 4;
}

// ============================================
// DETECTION DE CANAL OCCUPE (CAD)
// ============================================
bool LoRaDriver::isChannelBusy(void) {
    setMode(LORA_MODE_CAD);
    
    uint32_t timeout = millis() + 500;
    
    while (millis() < timeout) {
        uint8_t irqFlags = readRegister(REG_IRQ_FLAGS);
        
        if (irqFlags & IRQ_CAD_DONE_MASK) {
            writeRegister(REG_IRQ_FLAGS, IRQ_CAD_DONE_MASK);
            
            bool detected = (irqFlags & IRQ_CAD_DETECTED_MASK);
            setMode(LORA_MODE_STANDBY);
            return detected;
        }
        
        delay(1);
    }
    
    setMode(LORA_MODE_STANDBY);
    return false;
}

// ============================================
// CONTROLE DU MODULE
// ============================================
void LoRaDriver::sleep(void) {
    setMode(LORA_MODE_SLEEP);
}

void LoRaDriver::wakeup(void) {
    setMode(LORA_MODE_STANDBY);
}

void LoRaDriver::powerDown(void) {
    digitalWrite(LORA_RST_PIN, LOW);
    currentMode = LORA_MODE_SLEEP;
}

LoRaMode_t LoRaDriver::getMode(void) {
    return currentMode;
}

// ============================================
// STATISTIQUES
// ============================================
LoRaStats_t LoRaDriver::getStats(void) {
    return stats;
}

void LoRaDriver::resetStats(void) {
    memset(&stats, 0, sizeof(stats));
}

// ============================================
// DIAGNOSTIC
// ============================================
bool LoRaDriver::isAlive(void) {
    uint8_t version = getVersion();
    return (version == 0x12);
}

uint8_t LoRaDriver::getVersion(void) {
    return readRegister(REG_VERSION);
}

bool LoRaDriver::selfTest(void) {
    uint8_t original = readRegister(REG_SYNC_WORD);
    
    writeRegister(REG_SYNC_WORD, 0x55);
    uint8_t test = readRegister(REG_SYNC_WORD);
    
    writeRegister(REG_SYNC_WORD, original);
    
    return (test == 0x55);
}

// ============================================
// FIN DU FICHIER lora_driver.cpp
// ============================================