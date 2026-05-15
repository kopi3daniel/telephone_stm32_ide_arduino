
# 📡 Protocole de Communication - Téléphone LoRa

> **Version :** 1.0.0  
> **Date :** 2024  
> **Auteur :** [kopi3daniel](https://github.com/kopi3daniel)  
> **Dépôt :** [github.com/kopi3daniel](https://github.com/kopi3daniel)  
> **Cible :** Communication radio LoRa entre téléphones



## 📋 Table des matières

1. [Vue d'ensemble](#vue-densemble)
2. [Couches du protocole](#couches-du-protocole)
3. [Format des paquets](#format-des-paquets)
4. [Types de paquets](#types-de-paquets)
5. [Protocole d'appel](#protocole-dappel)
6. [Protocole SMS](#protocole-sms)
7. [Protocole de routage](#protocole-de-routage)
8. [Protocole de présence](#protocole-de-présence)
9. [Sécurité et chiffrement](#sécurité-et-chiffrement)
10. [Gestion des erreurs](#gestion-des-erreurs)
11. [Paramètres radio](#paramètres-radio)
12. [Exemples](#exemples)

---

## Vue d'ensemble

### Objectif

Ce protocole définit les règles de communication entre téléphones LoRa. Il permet :
- Les appels vocaux half-duplex
- L'envoi de messages texte (SMS)
- La découverte des autres téléphones
- Le routage via des nœuds intermédiaires
- La sécurisation optionnelle des échanges

### Caractéristiques

| Caractéristique | Valeur |
|----------------|--------|
| Support physique | LoRa (SX1278) |
| Fréquences | 868 MHz (EU), 915 MHz (US), 433 MHz (AS) |
| Modulation | LoRa (Chirp Spread Spectrum) |
| Débit | 1.8 - 27 kbps |
| Portée | 5 - 15 km (ligne de vue) |
| Adressage | ID 16 bits (0x0001 - 0xFFFE) |
| Broadcast | 0xFFFF |
| MTU | 255 octets |
| Half-duplex | Oui (un seul émetteur à la fois) |



## Couches du protocole


┌─────────────────────────────────────────────────────┐
│                   APPLICATION                        │
│         (Appels, SMS, Contacts, Fichiers)            │
├─────────────────────────────────────────────────────┤
│                   TRANSPORT                          │
│     (Fragmentation, ACK, Retransmission)             │
├─────────────────────────────────────────────────────┤
│                    ROUTAGE                           │
│         (DNS distribué, Table de routage)            │
├─────────────────────────────────────────────────────┤
│                   SÉCURITÉ                           │
│        (AES-128 optionnel, Authentification)         │
├─────────────────────────────────────────────────────┤
│                LIAISON DE DONNÉES                    │
│        (Framing, CRC, Détection d'erreurs)           │
├─────────────────────────────────────────────────────┤
│                    PHYSIQUE                          │
│         (LoRa SX1278, 868 MHz, SF/BW/CR)            │
└─────────────────────────────────────────────────────┘




## Format des paquets

### Structure générale

Tous les paquets commencent par un en-tête fixe de 8 octets.


┌────────────┬────────────┬──────────────┬──────────────┬─────────────┬────────────┬──────────────┐
│ Sync Byte  │ Packet Type│ Sender ID    │ Target ID    │ Sequence    │ Payload    │ Payload      │
│ 1 octet    │ 1 octet    │ 2 octets     │ 2 octets     │ Number      │ Size       │ Data         │
│            │            │              │              │ 1 octet     │ 1 octet    │ 0-247 octets │
└────────────┴────────────┴──────────────┴──────────────┴─────────────┴────────────┴──────────────┘


### Description des champs

| Champ | Taille | Description | Valeurs |
|-------|--------|-------------|---------|
| **Sync Byte** | 1 | Synchronisation | Toujours `0xAA` |
| **Packet Type** | 1 | Type de paquet | Voir tableau ci-dessous |
| **Sender ID** | 2 | ID de l'émetteur | 0x0001 - 0xFFFE |
| **Target ID** | 2 | ID du destinataire | 0x0001 - 0xFFFE, 0xFFFF = broadcast |
| **Sequence Number** | 1 | Numéro de séquence | 0-255 |
| **Payload Size** | 1 | Taille du payload | 0-247 |
| **Payload Data** | 0-247 | Données utiles | Dépend du type |



## Types de paquets

### Tableau complet des types

| Code | Nom | Description | Priorité | ACK requis | Mode radio |
|------|-----|-------------|----------|------------|------------|
| `0x10` | `CALL_REQUEST` | Demande d'appel | Haute | Oui | Data |
| `0x11` | `CALL_RINGING` | Sonnerie en cours | Haute | Non | Data |
| `0x12` | `CALL_ACCEPT` | Appel accepté | Haute | Oui | Data |
| `0x13` | `CALL_REJECT` | Appel refusé | Haute | Non | Data |
| `0x14` | `CALL_END` | Fin d'appel | Haute | Oui | Data |
| `0x15` | `CALL_BUSY` | Occupé | Haute | Non | Data |
| `0x20` | `VOICE_DATA` | Données vocales | Très haute | Non | Voix |
| `0x30` | `SMS_TEXT` | Message texte | Moyenne | Oui | Data |
| `0x31` | `SMS_ACK` | ACK message | Moyenne | Non | Data |
| `0x40` | `PING` | Test connectivité | Basse | Non | Data |
| `0x41` | `PONG` | Réponse ping | Basse | Non | Data |
| `0x50` | `PRESENCE` | Annonce présence | Basse | Non | Data |
| `0x60` | `DNS_REQUEST` | Résolution numéro→ID | Moyenne | Non | Data |
| `0x61` | `DNS_RESPONSE` | Réponse DNS | Moyenne | Non | Data |



## Protocole d'appel

### Diagramme de séquence


ÉMETTEUR (Alice)                          RÉCEPTEUR (Bob)
     │                                         │
     │ ════════ PHASE 1 : ÉTABLISSEMENT ═══════│
     │                                         │
     │ 1. CALL_REQUEST ──────────────────────→ │
     │ 2. ←────────────── CALL_RINGING ────────│
     │                                         │
     │ ════════ PHASE 2 : CONNEXION ═══════════│
     │                                         │
     │ 3. ←────────────── CALL_ACCEPT ─────────│
     │ 4. Bascule en mode VOIX (SF7/BW250)    │
     │                                         │
     │ ════════ PHASE 3 : COMMUNICATION ═══════│
     │                                         │
     │ 5. VOICE_DATA ←───────────────────────→ │
     │    (Échange half-duplex)                │
     │                                         │
     │ ════════ PHASE 4 : TERMINAISON ═════════│
     │                                         │
     │ 6. CALL_END ←─────────────────────────→ │
     │ 7. Retour mode DATA (SF9/BW125)        │


### États d'un appel


IDLE → DIALING → RINGING_REMOTE → ACTIVE → ENDING → ENDED
                    ▲                │
                    │                │
               RINGING_LOCAL ◄───────┘




## Protocole SMS

### Envoi d'un message simple (≤ 160 caractères)


ÉMETTEUR                                RÉCEPTEUR
    │                                       │
    │ 1. SMS_TEXT ────────────────────────→ │
    │ 2. ←──────────────── SMS_ACK ──────── │
    │ 3. Confirmation "Envoyé"              │


### Envoi d'un message long (> 160 caractères)


ÉMETTEUR                                RÉCEPTEUR
    │                                       │
    │ 1. SMS_TEXT (frag=1/3) ─────────────→ │
    │ 2. SMS_TEXT (frag=2/3) ─────────────→ │
    │ 3. SMS_TEXT (frag=3/3) ─────────────→ │
    │                                       │ 4. Assemble les fragments
    │ 5. ←─────────────── SMS_DELIVERY ──── │



## Paramètres radio

### Configuration Voix (faible latence)

| Paramètre | Valeur |
|-----------|--------|
| Fréquence | 868.0 MHz |
| Spreading Factor | 7 |
| Bande passante | 250 kHz |
| Coding Rate | 4/5 |
| Puissance TX | 17 dBm |
| Débit effectif | ~5.5 kbps |
| Latence | ~80ms |
| Portée | ~5 km |

### Configuration Data (longue portée)

| Paramètre | Valeur |
|-----------|--------|
| Fréquence | 868.0 MHz |
| Spreading Factor | 9 |
| Bande passante | 125 kHz |
| Coding Rate | 4/8 |
| Puissance TX | 17 dBm |
| Débit effectif | ~1.8 kbps |
| Latence | ~300ms |
| Portée | ~15 km |



## Exemples

### Exemple : Appel complet (hexadécimal)


CALL_REQUEST (Alice → Bob) :
AA 10 00 01 00 02 01 10 30 30 30 31 00 41 6C 69 63 65

CALL_RINGING (Bob → Alice) :
AA 11 00 02 00 01 01 00

CALL_ACCEPT (Bob → Alice) :
AA 12 00 02 00 01 02 00




## 📚 Références

- [Architecture du projet](ARCHITECTURE.md)
- [Datasheet SX1278](https://www.semtech.com/products/wireless-rf/lora-connect/sx1278)



## Licence et Contact

- **Auteur** : [kopi3daniel](https://github.com/kopi3daniel)
- **Dépôt** : [github.com/kopi3daniel](https://github.com/kopi3daniel)
- **Licence** : MIT License



> *Documentation générée pour le projet Téléphone LoRa*
> *Communications longue portée sans infrastructure*



