/**
 * ---------------------------------------------------------------------------
 * routing.cpp - Implementation du routage et decouverte reseau
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 */

#include "routing.h"
#include "lora_driver.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Reference externe au module LoRa
extern LoRaDriver lora;

// ============================================
// CONSTRUCTEUR
// ============================================
RoutingManager::RoutingManager() {
    neighborCount = 0;
    routeCount = 0;
    dnsCacheIndex = 0;
    lastPresenceAnnounce = 0;
    lastNeighborCleanup = 0;
    
    memset(neighbors, 0, sizeof(neighbors));
    memset(routes, 0, sizeof(routes));
    memset(dnsCache, 0, sizeof(dnsCache));
}

void RoutingManager::init(void) {
    // Annoncer notre presence immediatement
    announcePresence();
    lastPresenceAnnounce = millis();
}

// ============================================
// BOUCLE PRINCIPALE
// ============================================
void RoutingManager::process(void) {
    uint32_t now = millis();
    
    // Annoncer la presence periodiquement (toutes les 30 secondes)
    if ((now - lastPresenceAnnounce) / 1000 > PRESENCE_INTERVAL) {
        announcePresence();
        lastPresenceAnnounce = now;
    }
    
    // Nettoyer les voisins expires (toutes les 60 secondes)
    if ((now - lastNeighborCleanup) / 1000 > 60) {
        cleanupNeighbors();
        cleanupDNSCache();
        lastNeighborCleanup = now;
    }
}

// ============================================
// ANNONCE DE PRESENCE
// ============================================
bool RoutingManager::announcePresence(void) {
    
    LoRaPacket_t packet;
    packet.syncByte = PKT_SYNC_BYTE;
    packet.packetType = PKT_PRESENCE;
    packet.senderId = PHONE_ID;
    packet.targetId = 0xFFFF;  // Broadcast
    packet.sequenceNum = 0;
    
    // Payload : [numero(16)][nom(32)][type(1)][puissance(1)]
    uint8_t offset = 0;
    
    // Numero de telephone
    strncpy((char*)packet.payload + offset, PHONE_NUMBER, 16);
    offset += strlen(PHONE_NUMBER) + 1;
    
    // Surnom
    strncpy((char*)packet.payload + offset, PHONE_NAME, 32);
    offset += strlen(PHONE_NAME) + 1;
    
    // Type d appareil (1 = telephone)
    packet.payload[offset++] = 0x01;
    
    // Puissance d emission
    packet.payload[offset++] = LORA_VOICE_TX_POWER;
    
    packet.payloadSize = offset;
    
    return lora.sendPacket(&packet);
}

// ============================================
// PING / PONG
// ============================================
bool RoutingManager::sendPing(uint16_t targetId) {
    LoRaPacket_t packet;
    packet.syncByte = PKT_SYNC_BYTE;
    packet.packetType = PKT_PING;
    packet.senderId = PHONE_ID;
    packet.targetId = targetId;
    packet.sequenceNum = 0;
    packet.payloadSize = 4;
    
    // Horodatage pour mesurer la latence
    uint32_t timestamp = millis();
    memcpy(packet.payload, &timestamp, 4);
    
    return lora.sendPacket(&packet);
}

bool RoutingManager::sendPong(uint16_t targetId) {
    LoRaPacket_t packet;
    packet.syncByte = PKT_SYNC_BYTE;
    packet.packetType = PKT_PONG;
    packet.senderId = PHONE_ID;
    packet.targetId = targetId;
    packet.sequenceNum = 0;
    packet.payloadSize = 0;
    
    return lora.sendPacket(&packet);
}

// ============================================
// RESOLUTION DNS
// ============================================
bool RoutingManager::sendDNSRequest(const char* number) {
    LoRaPacket_t packet;
    packet.syncByte = PKT_SYNC_BYTE;
    packet.packetType = PKT_CONTACT_REQUEST;
    packet.senderId = PHONE_ID;
    packet.targetId = 0xFFFF;  // Broadcast
    packet.sequenceNum = 0;
    
    strncpy((char*)packet.payload, number, 16);
    packet.payloadSize = strlen(number) + 1;
    
    return lora.sendPacket(&packet);
}

bool RoutingManager::sendDNSResponse(uint16_t requesterId, const char* number, uint16_t phoneId) {
    LoRaPacket_t packet;
    packet.syncByte = PKT_SYNC_BYTE;
    packet.packetType = PKT_CONTACT_SEND;
    packet.senderId = PHONE_ID;
    packet.targetId = requesterId;
    packet.sequenceNum = 0;
    
    uint8_t offset = 0;
    strncpy((char*)packet.payload, number, 16);
    offset += strlen(number) + 1;
    
    packet.payload[offset++] = (phoneId >> 8) & 0xFF;
    packet.payload[offset++] = phoneId & 0xFF;
    
    packet.payloadSize = offset;
    
    return lora.sendPacket(&packet);
}

// ============================================
// RESOLUTION DE NUMERO
// ============================================
uint16_t RoutingManager::resolveNumber(const char* number) {
    
    // Etape 1 : Resolution locale (numero hex)
    uint16_t id = localResolve(number);
    if (id != 0) {
        return id;
    }
    
    // Etape 2 : Chercher dans le cache DNS
    id = lookupDNSCache(number);
    if (id != 0) {
        return id;
    }
    
    // Etape 3 : Chercher dans les voisins connus
    Neighbor_t* neighbor = findNeighborByNumber(number);
    if (neighbor != NULL && neighbor->state == NEIGHBOR_ONLINE) {
        cacheDNSResult(number, neighbor->phoneId);
        return neighbor->phoneId;
    }
    
    // Etape 4 : Broadcast DNS sur le reseau
    id = networkResolve(number);
    if (id != 0) {
        cacheDNSResult(number, id);
        return id;
    }
    
    return 0;  // Non trouve
}

uint16_t RoutingManager::localResolve(const char* number) {
    // Tenter une conversion hexadecimale directe
    // Exemple : "0002" -> 0x0002
    char* endPtr;
    uint16_t id = (uint16_t)strtol(number, &endPtr, 16);
    
    // Verifier que toute la chaine a ete convertie
    if (*endPtr == '\0') {
        return id;
    }
    
    return 0;
}

uint16_t RoutingManager::networkResolve(const char* number) {
    
    // Envoyer la requete DNS
    if (!sendDNSRequest(number)) {
        return 0;
    }
    
    // Attendre les reponses (timeout 2 secondes)
    uint32_t startTime = millis();
    
    while ((millis() - startTime) < DNS_TIMEOUT) {
        // Verifier si un paquet est arrive
        if (lora.isPacketAvailable()) {
            LoRaPacket_t packet;
            if (lora.receivePacket(&packet)) {
                // Verifier si c est une reponse DNS
                if (packet.packetType == PKT_CONTACT_SEND) {
                    char respNumber[16];
                    uint8_t offset = 0;
                    
                    strncpy(respNumber, (char*)packet.payload, 16);
                    offset += strlen(respNumber) + 1;
                    
                    uint16_t respId = (packet.payload[offset] << 8) | packet.payload[offset + 1];
                    
                    if (strcmp(respNumber, number) == 0) {
                        return respId;
                    }
                }
            }
        }
        delay(50);
    }
    
    return 0;  // Timeout
}

// ============================================
// GESTION DES VOISINS
// ============================================
Neighbor_t* RoutingManager::addNeighbor(uint16_t id, const char* number, 
                                        const char* name, int8_t rssi) {
    // Verifier si le voisin existe deja
    Neighbor_t* existing = findNeighbor(id);
    if (existing != NULL) {
        // Mettre a jour
        existing->rssi = rssi;
        existing->lastSeen = millis();
        existing->state = NEIGHBOR_ONLINE;
        return existing;
    }
    
    // Verifier la capacite
    if (neighborCount >= MAX_NEIGHBORS) {
        // Supprimer le plus ancien voisin hors ligne
        cleanupNeighbors();
        if (neighborCount >= MAX_NEIGHBORS) {
            return NULL;  // Toujours plein
        }
    }
    
    // Ajouter le nouveau voisin
    Neighbor_t* newNeighbor = &neighbors[neighborCount];
    newNeighbor->phoneId = id;
    strncpy(newNeighbor->phoneNumber, number, 15);
    strncpy(newNeighbor->nickname, name, 31);
    newNeighbor->state = NEIGHBOR_ONLINE;
    newNeighbor->rssi = rssi;
    newNeighbor->snr = 0;
    newNeighbor->hopCount = 1;  // Direct
    newNeighbor->viaNeighbor = 0;
    newNeighbor->lastSeen = millis();
    newNeighbor->firstSeen = millis();
    newNeighbor->canRelay = false;
    
    neighborCount++;
    return newNeighbor;
}

bool RoutingManager::updateNeighbor(uint16_t id, int8_t rssi, uint8_t hops) {
    Neighbor_t* neighbor = findNeighbor(id);
    if (neighbor == NULL) return false;
    
    neighbor->rssi = rssi;
    neighbor->hopCount = hops;
    neighbor->lastSeen = millis();
    
    if (neighbor->state != NEIGHBOR_ONLINE) {
        neighbor->state = NEIGHBOR_ONLINE;
    }
    
    return true;
}

void RoutingManager::cleanupNeighbors(void) {
    uint32_t now = millis();
    
    for (int i = neighborCount - 1; i >= 0; i--) {
        uint32_t elapsed = (now - neighbors[i].lastSeen) / 1000;
        
        if (elapsed > NEIGHBOR_TIMEOUT) {
            // Voisin expire
            if (neighbors[i].state == NEIGHBOR_ONLINE) {
                neighbors[i].state = NEIGHBOR_AWAY;
            }
            
            if (elapsed > NEIGHBOR_TIMEOUT * 3) {
                // Supprimer completement
                for (int j = i; j < neighborCount - 1; j++) {
                    neighbors[j] = neighbors[j + 1];
                }
                neighborCount--;
            }
        }
    }
}

Neighbor_t* RoutingManager::findNeighbor(uint16_t id) {
    for (uint8_t i = 0; i < neighborCount; i++) {
        if (neighbors[i].phoneId == id) {
            return &neighbors[i];
        }
    }
    return NULL;
}

Neighbor_t* RoutingManager::findNeighborByNumber(const char* number) {
    for (uint8_t i = 0; i < neighborCount; i++) {
        if (strcmp(neighbors[i].phoneNumber, number) == 0) {
            return &neighbors[i];
        }
    }
    return NULL;
}

uint8_t RoutingManager::getNeighborCount(void) { return neighborCount; }

Neighbor_t* RoutingManager::getNeighbor(uint8_t index) {
    if (index < neighborCount) return &neighbors[index];
    return NULL;
}

uint8_t RoutingManager::getOnlineNeighborCount(void) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < neighborCount; i++) {
        if (neighbors[i].state == NEIGHBOR_ONLINE) count++;
    }
    return count;
}

// ============================================
// TESTS DE CONNECTIVITE
// ============================================
bool RoutingManager::canReachDirectly(uint16_t targetId) {
    Neighbor_t* neighbor = findNeighbor(targetId);
    return (neighbor != NULL && neighbor->state == NEIGHBOR_ONLINE);
}

uint8_t RoutingManager::measureLinkQuality(uint16_t targetId) {
    Neighbor_t* neighbor = findNeighbor(targetId);
    if (neighbor == NULL) return 0;
    
    // Qualite basee sur le RSSI
    // -50 dBm = excellent (100%)
    // -120 dBm = tres faible (0%)
    int16_t rssi = neighbor->rssi;
    if (rssi > -50) return 100;
    if (rssi < -120) return 0;
    
    return (uint8_t)((rssi + 120) * 100 / 70);
}

uint32_t RoutingManager::pingNeighbor(uint16_t targetId) {
    uint32_t startTime = millis();
    
    if (sendPing(targetId)) {
        // Attendre le pong (timeout 2 secondes)
        uint32_t timeout = millis() + 2000;
        
        while (millis() < timeout) {
            if (lora.isPacketAvailable()) {
                LoRaPacket_t packet;
                if (lora.receivePacket(&packet)) {
                    if (packet.packetType == PKT_PONG && 
                        packet.senderId == targetId) {
                        return millis() - startTime;
                    }
                }
            }
            delay(10);
        }
    }
    
    return 0;  // Timeout
}

// ============================================
// ROUTAGE
// ============================================
bool RoutingManager::addRoute(uint16_t destination, uint16_t nextHop, 
                             uint8_t hops, uint8_t quality) {
    if (routeCount >= MAX_ROUTES) return false;
    
    routes[routeCount].destinationId = destination;
    routes[routeCount].nextHopId = nextHop;
    routes[routeCount].totalHops = hops;
    routes[routeCount].quality = quality;
    routes[routeCount].lastUsed = 0;
    routes[routeCount].created = millis();
    routes[routeCount].active = true;
    
    routeCount++;
    return true;
}

Route_t* RoutingManager::findBestRoute(uint16_t destinationId) {
    Route_t* best = NULL;
    uint8_t bestQuality = 0;
    
    for (uint8_t i = 0; i < routeCount; i++) {
        if (routes[i].destinationId == destinationId && routes[i].active) {
            if (routes[i].quality > bestQuality || best == NULL) {
                best = &routes[i];
                bestQuality = routes[i].quality;
            }
        }
    }
    
    return best;
}

bool RoutingManager::isRouteValid(Route_t* route) {
    if (route == NULL || !route->active) return false;
    
    // Verifier que le prochain saut est toujours en ligne
    Neighbor_t* nextHop = findNeighbor(route->nextHopId);
    return (nextHop != NULL && nextHop->state == NEIGHBOR_ONLINE);
}

uint16_t RoutingManager::getNextHop(uint16_t destinationId) {
    // Verifier si on peut l atteindre directement
    Neighbor_t* neighbor = findNeighbor(destinationId);
    if (neighbor != NULL && neighbor->state == NEIGHBOR_ONLINE) {
        return destinationId;  // Direct
    }
    
    // Chercher une route
    Route_t* route = findBestRoute(destinationId);
    if (route != NULL && isRouteValid(route)) {
        return route->nextHopId;
    }
    
    return 0;  // Pas de route
}

uint8_t RoutingManager::getHopCount(uint16_t destinationId) {
    Route_t* route = findBestRoute(destinationId);
    if (route != NULL) return route->totalHops;
    return 0;
}

bool RoutingManager::isReachable(uint16_t destinationId) {
    return (getNextHop(destinationId) != 0);
}

Route_t* RoutingManager::getBestRoute(uint16_t destinationId) {
    return findBestRoute(destinationId);
}

// ============================================
// CACHE DNS
// ============================================
void RoutingManager::cacheDNSResult(const char* number, uint16_t phoneId) {
    strncpy(dnsCache[dnsCacheIndex].number, number, 15);
    dnsCache[dnsCacheIndex].phoneId = phoneId;
    dnsCache[dnsCacheIndex].timestamp = millis();
    dnsCache[dnsCacheIndex].ttl = DNS_CACHE_TTL;
    
    dnsCacheIndex = (dnsCacheIndex + 1) % MAX_DNS_CACHE;
}

uint16_t RoutingManager::lookupDNSCache(const char* number) {
    uint32_t now = millis();
    
    for (uint8_t i = 0; i < MAX_DNS_CACHE; i++) {
        if (dnsCache[i].phoneId != 0 && 
            strcmp(dnsCache[i].number, number) == 0) {
            // Verifier le TTL
            if ((now - dnsCache[i].timestamp) / 1000 < dnsCache[i].ttl) {
                return dnsCache[i].phoneId;
            }
        }
    }
    return 0;
}

void RoutingManager::cleanupDNSCache(void) {
    uint32_t now = millis();
    
    for (uint8_t i = 0; i < MAX_DNS_CACHE; i++) {
        if (dnsCache[i].phoneId != 0) {
            if ((now - dnsCache[i].timestamp) / 1000 > dnsCache[i].ttl) {
                dnsCache[i].phoneId = 0;
            }
        }
    }
}

// ============================================
// TRAITEMENT DES PAQUETS RESEAU
// ============================================
void RoutingManager::handlePresence(uint16_t senderId, uint8_t* payload, uint8_t size) {
    // Extraire les informations
    char number[16] = {0};
    char name[32] = {0};
    uint8_t offset = 0;
    
    if (offset < size) {
        strncpy(number, (char*)payload + offset, 15);
        offset += strlen(number) + 1;
    }
    
    if (offset < size) {
        strncpy(name, (char*)payload + offset, 31);
        offset += strlen(name) + 1;
    }
    
    int8_t rssi = lora.getRSSI();
    
    // Ajouter ou mettre a jour le voisin
    addNeighbor(senderId, number, name, rssi);
}

void RoutingManager::handlePing(uint16_t senderId) {
    // Repondre immediatement par un pong
    sendPong(senderId);
}

void RoutingManager::handlePong(uint16_t senderId, int8_t rssi) {
    // Mettre a jour les informations du voisin
    updateNeighbor(senderId, rssi, 1);
}

void RoutingManager::handleDNSRequest(uint16_t requesterId, const char* number) {
    // Verifier si c est nous
    if (strcmp(number, PHONE_NUMBER) == 0) {
        sendDNSResponse(requesterId, number, PHONE_ID);
        return;
    }
    
    // Chercher dans nos voisins
    Neighbor_t* neighbor = findNeighborByNumber(number);
    if (neighbor != NULL) {
        sendDNSResponse(requesterId, number, neighbor->phoneId);
    }
}

void RoutingManager::handleDNSResponse(uint16_t responderId, const char* number, uint16_t phoneId) {
    cacheDNSResult(number, phoneId);
}

// ============================================
// INFORMATIONS RESEAU
// ============================================
uint8_t RoutingManager::getNetworkSize(void) {
    return neighborCount;
}

void RoutingManager::getNetworkStats(uint8_t* totalNodes, uint8_t* onlineNodes, uint8_t* routesCount) {
    *totalNodes = neighborCount;
    *onlineNodes = getOnlineNeighborCount();
    *routesCount = routeCount;
}

void RoutingManager::printRoutingTable(void) {
    // Debug : afficher sur le port serie
    DEBUG_SERIAL.println("=== TABLE DE ROUTAGE ===");
    DEBUG_SERIAL.print("Voisins : ");
    DEBUG_SERIAL.println(neighborCount);
    for (uint8_t i = 0; i < neighborCount; i++) {
        DEBUG_SERIAL.print("  ");
        DEBUG_SERIAL.print(neighbors[i].phoneId, HEX);
        DEBUG_SERIAL.print(": ");
        DEBUG_SERIAL.print(neighbors[i].nickname);
        DEBUG_SERIAL.print(" (");
        DEBUG_SERIAL.print(neighbors[i].state == NEIGHBOR_ONLINE ? "ONLINE" : "OFFLINE");
        DEBUG_SERIAL.print(") RSSI:");
        DEBUG_SERIAL.println(neighbors[i].rssi);
    }
    DEBUG_SERIAL.print("Routes : ");
    DEBUG_SERIAL.println(routeCount);
    DEBUG_SERIAL.println("=========================");
}

void RoutingManager::clearAll(void) {
    memset(neighbors, 0, sizeof(neighbors));
    memset(routes, 0, sizeof(routes));
    memset(dnsCache, 0, sizeof(dnsCache));
    neighborCount = 0;
    routeCount = 0;
    dnsCacheIndex = 0;
}

void RoutingManager::reset(void) {
    clearAll();
    init();
}

// ============================================
// FIN DU FICHIER routing.cpp
// ============================================