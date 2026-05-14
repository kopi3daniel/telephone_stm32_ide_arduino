/**
 * ---------------------------------------------------------------------------
 * routing.h - Routage et decouverte reseau pour Telephone LoRa
 * (Version Arduino IDE - STM32F103C8T6 Blue Pill)
 * ---------------------------------------------------------------------------
 * 
 * Ce fichier declare la classe RoutingManager qui gere :
 * - La decouverte des autres telephones sur le reseau
 * - La table de routage (numero -> ID -> chemin)
 * - La resolution DNS distribuee (numero -> ID)
 * - Les pings pour tester la connectivite
 * - La gestion des sauts (hops) pour les futurs relais
 * - Les annonces de presence periodiques
 */

#ifndef ROUTING_H
#define ROUTING_H

#include <Arduino.h>
#include "config.h"

// ============================================
// NOMBRE MAXIMUM D ENTREES DANS LES TABLES
// ============================================
#define MAX_NEIGHBORS       20      // Voisins directs connus
#define MAX_ROUTES          30      // Routes completes
#define MAX_DNS_CACHE       50      // Cache DNS (numero -> ID)

// ============================================
// ETAT D UN VOISIN
// ============================================
typedef enum {
    NEIGHBOR_UNKNOWN,       // Jamais vu
    NEIGHBOR_ONLINE,        // En ligne et joignable
    NEIGHBOR_AWAY,          // Vu recemment mais plus joignable
    NEIGHBOR_OFFLINE        // Hors ligne confirme
} NeighborState_t;

// ============================================
// STRUCTURE D UN VOISIN
// ============================================
typedef struct {
    uint16_t phoneId;           // ID unique du voisin
    char phoneNumber[16];       // Numero de telephone
    char nickname[32];          // Surnom ou nom
    NeighborState_t state;      // Etat actuel
    int8_t rssi;               // Force du signal (dBm)
    int8_t snr;                // Rapport signal/bruit (dB)
    uint8_t hopCount;           // Nombre de sauts pour l atteindre
    uint16_t viaNeighbor;       // ID du voisin par lequel passer (0 = direct)
    uint32_t lastSeen;          // Derniere fois qu on l a vu
    uint32_t firstSeen;         // Premiere fois qu on l a vu
    bool canRelay;              // Peut servir de relais ?
} Neighbor_t;

// ============================================
// STRUCTURE D UNE ROUTE
// ============================================
typedef struct {
    uint16_t destinationId;     // ID de la destination finale
    uint16_t nextHopId;         // Prochain saut (voisin direct)
    uint8_t totalHops;          // Nombre total de sauts
    uint8_t quality;            // Qualite de la route (0-100)
    uint32_t lastUsed;          // Derniere utilisation
    uint32_t created;           // Date de creation
    bool active;                // Route active
} Route_t;

// ============================================
// STRUCTURE DU CACHE DNS
// ============================================
typedef struct {
    char number[16];            // Numero recherche
    uint16_t phoneId;           // ID resolu
    uint32_t timestamp;         // Quand la resolution a ete faite
    uint8_t ttl;               // Duree de vie (secondes)
} DNSCache_t;

// ============================================
// CLASSE PRINCIPALE
// ============================================
class RoutingManager {
    
private:
    // ============================================
    // TABLES DE DONNEES
    // ============================================
    
    // Table des voisins connus
    Neighbor_t neighbors[MAX_NEIGHBORS];
    uint8_t neighborCount;
    
    // Table de routage
    Route_t routes[MAX_ROUTES];
    uint8_t routeCount;
    
    // Cache DNS
    DNSCache_t dnsCache[MAX_DNS_CACHE];
    uint8_t dnsCacheIndex;
    
    // ============================================
    // CONFIGURATION
    // ============================================
    
    // Intervalle d annonce de presence (secondes)
    static const uint32_t PRESENCE_INTERVAL = 30;
    
    // Timeout pour considerer un voisin perdu (secondes)
    static const uint32_t NEIGHBOR_TIMEOUT = 120;
    
    // Timeout pour une requete DNS (ms)
    static const uint32_t DNS_TIMEOUT = 2000;
    
    // TTL du cache DNS (secondes)
    static const uint8_t DNS_CACHE_TTL = 120;
    
    // Nombre maximum de sauts pour une route
    static const uint8_t MAX_HOPS = 5;
    
    // ============================================
    // HORODATAGES
    // ============================================
    uint32_t lastPresenceAnnounce;  // Derniere annonce de presence
    uint32_t lastNeighborCleanup;   // Dernier nettoyage des voisins
    
    // ============================================
    // METHODES PRIVEES
    // ============================================
    
    // Envoie une annonce de presence
    bool announcePresence(void);
    
    // Envoie un ping a un voisin
    bool sendPing(uint16_t targetId);
    
    // Envoie une reponse pong
    bool sendPong(uint16_t targetId);
    
    // Envoie une requete DNS broadcast
    bool sendDNSRequest(const char* number);
    
    // Envoie une reponse DNS
    bool sendDNSResponse(uint16_t requesterId, const char* number, uint16_t phoneId);
    
    // Ajoute un voisin a la table
    Neighbor_t* addNeighbor(uint16_t id, const char* number, const char* name, int8_t rssi);
    
    // Met a jour les informations d un voisin existant
    bool updateNeighbor(uint16_t id, int8_t rssi, uint8_t hops);
    
    // Supprime les voisins trop anciens
    void cleanupNeighbors(void);
    
    // Ajoute une route a la table
    bool addRoute(uint16_t destination, uint16_t nextHop, uint8_t hops, uint8_t quality);
    
    // Trouve la meilleure route vers une destination
    Route_t* findBestRoute(uint16_t destinationId);
    
    // Verifie si une route est encore valide
    bool isRouteValid(Route_t* route);
    
    // Met en cache une resolution DNS
    void cacheDNSResult(const char* number, uint16_t phoneId);
    
    // Cherche dans le cache DNS
    uint16_t lookupDNSCache(const char* number);
    
    // Nettoie le cache DNS des entrees expirees
    void cleanupDNSCache(void);
    
public:
    // ============================================
    // CONSTRUCTEUR ET INITIALISATION
    // ============================================
    RoutingManager();
    void init(void);
    
    // ============================================
    // BOUCLE PRINCIPALE
    // ============================================
    void process(void);
    
    // ============================================
    // RESOLUTION NUMERO -> ID
    // ============================================
    
    // Resout un numero en ID (local + reseau)
    uint16_t resolveNumber(const char* number);
    
    // Resolution locale uniquement
    uint16_t localResolve(const char* number);
    
    // Resolution reseau (broadcast DNS)
    uint16_t networkResolve(const char* number);
    
    // ============================================
    // GESTION DES VOISINS
    // ============================================
    
    // Retourne le nombre de voisins connus
    uint8_t getNeighborCount(void);
    
    // Retourne un voisin par index
    Neighbor_t* getNeighbor(uint8_t index);
    
    // Trouve un voisin par ID
    Neighbor_t* findNeighbor(uint16_t id);
    
    // Trouve un voisin par numero
    Neighbor_t* findNeighborByNumber(const char* number);
    
    // Retourne le nombre de voisins en ligne
    uint8_t getOnlineNeighborCount(void);
    
    // ============================================
    // TESTS DE CONNECTIVITE
    // ============================================
    
    // Teste si un voisin est joignable directement
    bool canReachDirectly(uint16_t targetId);
    
    // Mesure la qualite de la connexion vers un voisin
    uint8_t measureLinkQuality(uint16_t targetId);
    
    // Ping un voisin pour tester la latence
    uint32_t pingNeighbor(uint16_t targetId);
    
    // ============================================
    // ROUTAGE
    // ============================================
    
    // Trouve le prochain saut pour atteindre une destination
    uint16_t getNextHop(uint16_t destinationId);
    
    // Calcule le nombre de sauts vers une destination
    uint8_t getHopCount(uint16_t destinationId);
    
    // Verifie si une destination est accessible
    bool isReachable(uint16_t destinationId);
    
    // Retourne la meilleure route
    Route_t* getBestRoute(uint16_t destinationId);
    
    // ============================================
    // TRAITEMENT DES PAQUETS RESEAU
    // ============================================
    
    // Traite un paquet de presence
    void handlePresence(uint16_t senderId, uint8_t* payload, uint8_t size);
    
    // Traite un ping entrant
    void handlePing(uint16_t senderId);
    
    // Traite un pong entrant
    void handlePong(uint16_t senderId, int8_t rssi);
    
    // Traite une requete DNS
    void handleDNSRequest(uint16_t requesterId, const char* number);
    
    // Traite une reponse DNS
    void handleDNSResponse(uint16_t responderId, const char* number, uint16_t phoneId);
    
    // ============================================
    // INFORMATIONS RESEAU
    // ============================================
    
    // Retourne le nombre de telephones sur le reseau
    uint8_t getNetworkSize(void);
    
    // Affiche la table de routage (debug)
    void printRoutingTable(void);
    
    // Retourne des statistiques reseau
    void getNetworkStats(uint8_t* totalNodes, uint8_t* onlineNodes, uint8_t* routes);
    
    // ============================================
    // NETTOYAGE
    // ============================================
    
    // Supprime toutes les entrees
    void clearAll(void);
    
    // Reinitialise les tables
    void reset(void);
};

#endif // ROUTING_H