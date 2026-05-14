Voici le fichier `ARCHITECTURE.md` corrigé avec vos informations GitHub :

```markdown
# 🏗️ Architecture du Téléphone LoRa

> **Version :** 1.0.0  
> **Date :** 2024  
> **Auteur :** [kopi3daniel](https://github.com/kopi3daniel)  
> **Dépôt :** [github.com/kopi3daniel](https://github.com/kopi3daniel)  
> **Cible :** STM32F103C8T6 (Blue Pill) + LoRa RA-02 (SX1278)

---

## 📋 Table des matières

1. [Vue d'ensemble](#vue-densemble)
2. [Architecture matérielle](#architecture-matérielle)
3. [Architecture logicielle](#architecture-logicielle)
4. [Modules et responsabilités](#modules-et-responsabilités)
5. [Flux de données](#flux-de-données)
6. [Machine d'états](#machine-détats)
7. [Protocoles de communication](#protocoles-de-communication)
8. [Gestion de l'énergie](#gestion-de-lénergie)
9. [Sécurité](#sécurité)
10. [Performances](#performances)

---

## Vue d'ensemble

### Description

Le Téléphone LoRa est un appareil de communication longue portée qui fonctionne **sans infrastructure** (pas de réseau mobile, WiFi ou Bluetooth). Il utilise la technologie LoRa pour transmettre la voix et les messages texte sur des distances de 5 à 15 km.

### Principes de conception

- **Autonomie totale** : Aucune dépendance à un opérateur télécom
- **Basse consommation** : Optimisé pour fonctionner plusieurs jours sur batterie
- **Modulaire** : Chaque fonction est isolée dans un module indépendant
- **Robuste** : Tolérance aux erreurs et gestion des cas limites
- **Simple** : Interface utilisateur inspirée des téléphones classiques

### Diagramme de contexte

```
┌─────────────────────────────────────────────────────────────┐
│                      ENVIRONNEMENT                           │
│                                                              │
│   ┌──────────┐                          ┌──────────┐        │
│   │  Alice   │                          │   Bob    │        │
│   │  (Appel) │◄──────── LoRa ──────────►│ (Répond) │        │
│   └──────────┘                          └──────────┘        │
│        │                                      │              │
│        │           Portée : 5-15 km            │              │
│        │           Fréquence : 868 MHz         │              │
│        │           Débit : 5-27 kbps          │              │
│        │                                      │              │
│   ┌────┴──────────────────────────────────────┴────┐        │
│   │              TÉLÉPHONE LORA                     │        │
│   │                                                  │        │
│   │  ┌──────────┐  ┌──────────┐  ┌──────────┐      │        │
│   │  │  Clavier │  │  Écran   │  │  Audio   │      │        │
│   │  │  4x6     │  │OLED 1.3" │  │ Mic+HP   │      │        │
│   │  └────┬─────┘  └────┬─────┘  └────┬─────┘      │        │
│   │       └──────────────┼──────────────┘            │        │
│   │                      │                           │        │
│   │              ┌───────┴───────┐                   │        │
│   │              │  STM32F103    │                   │        │
│   │              │   (Cerveau)   │                   │        │
│   │              └───────┬───────┘                   │        │
│   │                      │                           │        │
│   │              ┌───────┴───────┐                   │        │
│   │              │   SX1278      │                   │        │
│   │              │   (Radio)     │                   │        │
│   │              └───────────────┘                   │        │
│   └──────────────────────────────────────────────────┘        │
└─────────────────────────────────────────────────────────────┘
```

---

## Architecture matérielle

### Composants

| Composant | Rôle | Interface | Consommation |
|-----------|------|-----------|--------------|
| **STM32F103C8T6** | CPU principal | - | 36mA (actif) |
| **RA-02 (SX1278)** | Radio LoRa | SPI1 | 120mA (TX) |
| **OLED 1.3"** | Affichage | I2C1 | 20mA |
| **Clavier 4×6** | Saisie utilisateur | GPIO | - |
| **Microphone** | Capture audio | ADC1 | 1mA |
| **Haut-parleur** | Lecture audio | TIM1 PWM | 100mA |
| **LED Torche** | Éclairage | TIM2 PWM | 50mA |
| **Buzzer** | Sonnerie | GPIO | 30mA |
| **Vibreur** | Alerte silencieuse | GPIO | 80mA |
| **Batterie Li-Ion** | Alimentation | ADC1 | 2000mAh |

### Brochage détaillé

```
STM32F103C8T6 (Blue Pill) - 48 pins

┌─────────────────────────────────────────────────────┐
│                                                     │
│  PORTA                                              │
│  PA0  → LoRa DIO0 (Interruption)                    │
│  PA1  → Microphone (ADC1_CH1)                       │
│  PA2  → LoRa RESET                                  │
│  PA4  → LoRa NSS (SPI1)                             │
│  PA5  → LoRa SCK (SPI1)                             │
│  PA6  → LoRa MISO (SPI1)                            │
│  PA7  → LoRa MOSI (SPI1)                            │
│  PA8  → Haut-parleur (TIM1_CH1)                     │
│  PA9  → Clavier Ligne 5                             │
│  PA10 → Clavier Colonne 4                           │
│                                                     │
│  PORTB                                              │
│  PB3  → Clavier Colonne 1                           │
│  PB4  → Clavier Colonne 2                           │
│  PB5  → Clavier Colonne 3                           │
│  PB6  → OLED SCL (I2C1)                             │
│  PB7  → OLED SDA (I2C1)                             │
│  PB8  → Bouton APPEL (VERT)                         │
│  PB9  → Bouton RACCROCHER (ROUGE)                   │
│  PB12 → Clavier Ligne 1                             │
│  PB13 → Clavier Ligne 2                             │
│  PB14 → Clavier Ligne 3                             │
│  PB15 → Clavier Ligne 4                             │
│                                                     │
│  PORTC                                              │
│  PC13 → LED Torche                                  │
│  PC14 → Vibreur                                     │
│  PC15 → Buzzer                                      │
│                                                     │
└─────────────────────────────────────────────────────┘
```

---

## Architecture logicielle

### Structure en couches

```
┌─────────────────────────────────────────────────────────────┐
│                     APPLICATION (main.cpp)                   │
│                    Machine d'états globale                    │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌─────────────────┐  ┌─────────────────┐                   │
│  │   INTERFACE     │  │   PROTOCOLES    │                   │
│  │   UTILISATEUR   │  │                  │                   │
│  │                  │  │  CallManager    │                   │
│  │  Screens :       │  │  SMSManager     │                   │
│  │  - HomeScreen    │  │  RoutingManager │                   │
│  │  - DialerScreen  │  │  CryptoManager  │                   │
│  │  - CallScreen    │  │                  │                   │
│  │  - ContactsScreen│  └─────────────────┘                   │
│  │  - SMSScreen     │                                        │
│  │  - SettingsScreen│                                        │
│  │                  │                                        │
│  │  Widgets :       │                                        │
│  │  - StatusBar     │                                        │
│  │  - Menu          │                                        │
│  │  - Dialog        │                                        │
│  │  - InputField    │                                        │
│  │  - Icons         │                                        │
│  └─────────────────┘                                        │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌─────────────────┐  ┌─────────────────┐                   │
│  │    DRIVERS      │  │   UTILITAIRES   │                   │
│  │                  │  │                  │                   │
│  │  Keyboard        │  │  Timers          │                   │
│  │  Display         │  │  Compression     │                   │
│  │  LoRaDriver      │  │  Storage         │                   │
│  │  Audio           │  │  CRC             │                   │
│  │  PowerManager    │  │  Buffers         │                   │
│  └─────────────────┘  └─────────────────┘                   │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│                     HAL STM32                                │
│    (SPI, I2C, ADC, TIM, GPIO, RCC, NVIC, DMA)              │
├─────────────────────────────────────────────────────────────┤
│                     HARDWARE                                 │
│    (STM32F103C8T6, SX1278, SSD1306, Clavier, Audio)       │
└─────────────────────────────────────────────────────────────┘
```

### Arborescence des fichiers

```
LoRaPhone/
│
├── 📁 docs/                             # Documentation
│   └── 📄 ARCHITECTURE.md               # Ce document
│
├── 📁 src/                              # Code source
│   ├── 📄 config.h                      # Configuration globale
│   ├── 📄 keyboard.h / .cpp             # Clavier 4×6 + multi-tap
│   ├── 📄 display.h / .cpp              # Écran OLED SSD1306
│   ├── 📄 lora_driver.h / .cpp          # Module LoRa SX1278
│   ├── 📄 audio.h / .cpp                # Micro + HP + ADPCM
│   ├── 📄 power_manager.h / .cpp        # Gestion batterie/énergie
│   ├── 📄 call_manager.h / .cpp         # Gestion des appels
│   ├── 📄 sms_manager.h / .cpp          # Gestion des SMS
│   ├── 📄 routing.h / .cpp              # Routage réseau maillé
│   ├── 📄 crypto.h / .cpp               # Chiffrement AES-128
│   ├── 📄 home_screen.h / .cpp          # Écran d'accueil
│   ├── 📄 dialer_screen.h / .cpp        # Composeur numérique
│   ├── 📄 call_screen.h / .cpp          # Appel en cours
│   ├── 📄 contacts_screen.h / .cpp      # Répertoire
│   ├── 📄 sms_screen.h / .cpp           # Messages
│   ├── 📄 settings_screen.h / .cpp      # Paramètres
│   ├── 📄 status_bar.h / .cpp           # Barre d'état
│   ├── 📄 menu.h / .cpp                 # Menu générique
│   ├── 📄 dialog.h / .cpp               # Boîtes de dialogue
│   ├── 📄 input_field.h / .cpp          # Champ de saisie
│   ├── 📄 icons.h / .cpp                # Bibliothèque d'icônes
│   ├── 📄 buffers.h / .cpp              # Buffers circulaires
│   ├── 📄 timers.h / .cpp               # Timers logiciels
│   ├── 📄 compression.h / .cpp          # Compression ADPCM
│   ├── 📄 storage.h / .cpp              # Stockage Flash
│   ├── 📄 crc.h / .cpp                  # Checksums
│   └── 📄 teleogone_lora.ino            # Point d'entrée Arduino
```

---

## Modules et responsabilités

### Drivers (Couche basse)

| Module | Responsabilité | Interfaces |
|--------|---------------|------------|
| **Keyboard** | Scanner la matrice 4×6, anti-rebond, multi-tap T9 | GPIO, Timers |
| **Display** | Afficher sur OLED 128×64, icônes, texte | I2C, SSD1306 |
| **LoRaDriver** | Communication radio SX1278, profils Voix/Data | SPI, DIO0 (IRQ) |
| **Audio** | Capture micro (ADC), lecture HP (PWM), ADPCM | ADC1, TIM1 |
| **PowerManager** | Mesure batterie, modes veille, autonomie | ADC1, RCC |

### Protocoles (Couche intermédiaire)

| Module | Responsabilité | Utilise |
|--------|---------------|--------|
| **CallManager** | Machine d'états des appels, signalisation, voix | LoRaDriver, Audio |
| **SMSManager** | Envoi/réception SMS, fragmentation, stockage | LoRaDriver, Storage |
| **RoutingManager** | Découverte réseau, DNS, table de routage | LoRaDriver |
| **CryptoManager** | Chiffrement AES-128, authentification | - |

### Interface utilisateur (Couche haute)

| Écran | Responsabilité |
|-------|---------------|
| **HomeScreen** | Accueil, horloge, icônes de menu |
| **DialerScreen** | Composition numéro, suggestions |
| **CallScreen** | Appel en cours, durée, options |
| **ContactsScreen** | Liste, vue, édition contacts |
| **SMSScreen** | Conversations, éditeur, envoi |
| **SettingsScreen** | Son, écran, réseau, sécurité |

### Utilitaires

| Module | Responsabilité |
|--------|---------------|
| **Timers** | Timers logiciels non-bloquants |
| **Compression** | ADPCM encode/décode (2:1) |
| **Storage** | Sauvegarde Flash (contacts, SMS, journal) |
| **CRC** | CRC8, CRC16, checksums |
| **Buffers** | Buffers circulaires, files d'attente |

---

## Flux de données

### Flux d'un appel vocal

```
MICROPHONE                    HAUT-PARLEUR
    │                              ▲
    ▼                              │
┌─────────┐                  ┌─────────┐
│   ADC   │                  │   PWM   │
│ 8kHz    │                  │ 8kHz    │
│ 8-bit   │                  │ 8-bit   │
└────┬────┘                  └────┬────┘
     │                            │
     ▼                            ▲
┌─────────┐                  ┌─────────┐
│ Buffer  │                  │ Buffer  │
│ 64 ech. │                  │ 64 ech. │
└────┬────┘                  └────┬────┘
     │                            │
     ▼                            ▲
┌─────────┐                  ┌─────────┐
│ ADPCM   │                  │ ADPCM   │
│ Encode  │                  │ Decode  │
│ 64→32   │                  │ 32→64   │
└────┬────┘                  └────┬────┘
     │                            │
     ▼                            ▲
┌─────────┐                  ┌─────────┐
│ Paquet  │                  │ Paquet  │
│ Voice   │    ┌─────────┐   │ Voice   │
│ (32 B)  │───►│  LoRa   │───►│ (32 B)  │
└─────────┘    │  Radio  │   └─────────┘
               └─────────┘
                    │
                    ▼
               ┌─────────┐
               │ Antenne │
               │ 868 MHz │
               └─────────┘
```

---

## Machine d'états

### États globaux du téléphone

```
                    ┌──────────────┐
                    │   POWER_ON   │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
              ┌─────│    HOME      │◄────────┐
              │     └──┬───┬───┬───┘         │
              │        │   │   │              │
              ▼        ▼   ▼   ▼              │
        ┌─────────┐ ┌───┐ ┌───┐ ┌──────┐     │
        │ DIALER  │ │SMS│ │CT │ │SETTINGS│    │
        └────┬────┘ └───┘ └───┘ └──────┘     │
             │                                │
             ▼                                │
        ┌─────────┐                          │
        │IN_CALL  │──────────────────────────┘
        └────┬────┘
             │
             ▼
        ┌─────────┐
        │CALL_END │──────────► HOME
        └─────────┘
```

---

## Gestion de l'énergie

### Modes de consommation

| Mode | CPU | Écran | LoRa | Conso. | Déclencheur |
|------|-----|-------|------|--------|-------------|
| **ACTIVE** | 72MHz | ON | RX continu | 120mA | Activité utilisateur |
| **SCREEN_OFF** | 72MHz | OFF | RX continu | 80mA | 30s inactivité |
| **SLEEP** | 8MHz | OFF | RX cyclique | 15mA | 5min inactivité |
| **STOP** | OFF | OFF | RX rare | 2mA | 1h inactivité |
| **STANDBY** | OFF | OFF | OFF | 5µA | Batterie < 3% |

### Autonomie estimée

| Usage | Autonomie |
|-------|-----------|
| Appel continu | ~16 heures |
| Normal (5 appels/jour) | ~3 jours |
| Veille légère | ~1 semaine |
| Veille profonde | ~1 mois |

---

## Performances

### Métriques clés

| Métrique | Valeur |
|----------|--------|
| Latence voix | ~80ms (SF7/BW250) |
| Portée voix | ~5 km (SF7) |
| Portée SMS | ~15 km (SF9) |
| Débit voix | ~5.5 kbps |
| Débit SMS | ~1.8 kbps |
| Temps établissement appel | ~500ms |
| Temps envoi SMS | ~200ms |
| Occupation CPU | ~40% (actif) |
| RAM utilisée | ~12 KB / 20 KB |
| Flash utilisée | ~52 KB / 64 KB |

---

## Licence et Contact

- **Auteur** : [kopi3daniel](https://github.com/kopi3daniel)
- **Dépôt** : [github.com/kopi3daniel](https://github.com/kopi3daniel)
- **Licence** : MIT License

---

> *Documentation générée avec ❤️ pour la communauté open-source*
> *Projet Téléphone LoRa - Communications longue portée sans infrastructure*
```

---
