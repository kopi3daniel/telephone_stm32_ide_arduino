Voici un `README.md` complet et prêt à être publié sur votre GitHub :

---

## 📄 `README.md` complet pour GitHub

```markdown
# 📱 Téléphone LoRa - Communication longue portée sans réseau mobile

> **Auteur :** [kopi3daniel (Daniel)](https://github.com/kopi3daniel)  
> **Version :** 1.0.0  
> **Licence :** MIT  
> **Cible :** STM32F103C8T6 (Blue Pill) + RA-02 (SX1278)  
> **Environnement :** Arduino IDE

---

## 🎯 Présentation

Le **Téléphone LoRa** est un téléphone portable qui fonctionne **sans réseau mobile** (pas de 4G/5G, pas de WiFi, pas de Bluetooth). Il utilise la technologie radio **LoRa** (Long Range) pour transmettre la voix et les messages texte sur de **longues distances** (5 à 15 km).

Imaginez un téléphone qui fonctionnerait comme un talkie-walkie ultra perfectionné : vous pouvez passer des appels vocaux et envoyer des SMS, mais sans passer par les antennes des opérateurs téléphoniques.

### 🚀 Caractéristiques

- ✅ **Appels vocaux** half-duplex avec compression ADPCM
- ✅ **SMS** 160 caractères (fragmentation automatique)
- ✅ **Répertoire** de contacts (100 max) avec favoris
- ✅ **Journal d'appels** (entrants, sortants, manqués)
- ✅ **Interface utilisateur** complète (6 écrans)
- ✅ **Saisie texte multi-tap** (mode T9)
- ✅ **Écran OLED 1.3"** 128×64 pixels
- ✅ **Clavier 24 touches** matriciel 4×6
- ✅ **LED Torche** intégrée
- ✅ **Gestion batterie** intelligente (RUN/SLEEP/STOP/STANDBY)
- ✅ **Chiffrement AES-128** optionnel
- ✅ **Découverte réseau** automatique
- ✅ **Portée** 5-15 km en ligne de vue

---

## 📸 Aperçu

```
┌──────────────────────────┐
│ ▂▂▂▂ 14:30    ████ 85%  │ ← Barre d'état
├──────────────────────────┤
│                          │
│      12:34               │
│    Lun 15 Jan            │
│                          │
│  [📞] [✉️] [📇] [⚙️]    │
│  Appels SMS Contacts Regl.│
└──────────────────────────┘

┌──────────────────────────┐
│   Appel avec BOB         │
│                          │
│       ☎️ 🟢              │
│    00:02:34              │
│                          │
│  [🎤 Muet] [🔊 HP]       │
│  [❌ Raccrocher]         │
└──────────────────────────┘
```

---

## 🛠️ Matériel requis

| Composant | Modèle | Quantité | Prix approx. |
|-----------|--------|----------|-------------|
| Microcontrôleur | STM32F103C8T6 (Blue Pill) | 1 | ~3€ |
| Module LoRa | RA-02 (SX1278) 868MHz | 1 | ~8€ |
| Écran OLED | 1.3" SSD1306 128×64 I2C | 1 | ~4€ |
| Régulateur | AMS1117-3.3V | 1 | ~0.5€ |
| Chargeur | TP4056 micro-USB | 1 | ~1€ |
| Batterie | Li-Ion 18650 2000mAh | 1 | ~5€ |
| Ampli audio | LM386 module | 1 | ~2€ |
| Haut-parleur | 8Ω 0.5W | 1 | ~1€ |
| Microphone | Électret | 1 | ~0.5€ |
| Clavier | 24 touches poussoir | 24 | ~3€ |
| Buzzer | Piézoélectrique | 1 | ~0.5€ |
| Vibreur | Mini moteur vibrant | 1 | ~1€ |
| Divers | Résistances, cond., fils... | - | ~5€ |
| **TOTAL** | | | **~35€** |

---

## 🔌 Schéma de câblage rapide

| Périphérique | Broches STM32 |
|-------------|---------------|
| **LoRa RA-02** | PA4 (NSS), PA5 (SCK), PA6 (MISO), PA7 (MOSI), PA0 (DIO0), PA2 (RST) |
| **OLED 1.3"** | PB6 (SCL), PB7 (SDA) - I2C1 |
| **Audio HP** | PA8 (TIM1 PWM) → LM386 → HP 8Ω |
| **Microphone** | PA1 (ADC1) |
| **Clavier Lignes** | PB12, PB13, PB14, PB15, PA9, PA8 |
| **Clavier Colonnes** | PB3, PB4, PB5, PA10 |
| **Bouton APPEL** | PB8 (pull-up) |
| **Bouton FIN** | PB9 (pull-up) |
| **LED Torche** | PC13 |
| **Buzzer** | PC15 |
| **Vibreur** | PC14 |

📖 **Schéma complet :** [docs/SCHEMATIC.md](docs/SCHEMATIC.md)

---

## ⚡ Démarrage rapide (Arduino IDE)

### 1. Installer le core STM32

1. Ouvrir l'IDE Arduino
2. **Fichier > Préférences** → Ajouter l'URL :
   ```
   https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
   ```
3. **Outils > Type de carte > Gestionnaire de cartes** → Installer **STM32 MCU based boards**
4. Sélectionner : **Generic STM32F1 series > BluePill F103C8**

### 2. Installer les bibliothèques

Dans **Outils > Gérer les bibliothèques** :
- **Keypad** (Mark Stanley, Alexander Brevig)
- **Adafruit SSD1306** (Adafruit)
- **Adafruit GFX Library** (Adafruit)

### 3. Compiler et flasher

```bash
# Cloner le dépôt
git clone https://github.com/kopi3daniel/LoRaPhone.git

# Ouvrir teleogone_lora.ino dans Arduino IDE

# Brancher la Blue Pill via USB-TTL
# Mettre le cavalier BOOT0 sur 1
# Compiler et téléverser
# Remettre BOOT0 sur 0
```

---

## 📁 Structure du projet

```
LoRaPhone/
├── 📄 telephone_lora.ino       # Programme principal Arduino
|   |__ config.h                 # Configuration globale (broches, fréquences...)
│   ├── keyboard.h/.cpp         # Clavier matriciel 4×6 + multi-tap
│   ├── display.h/.cpp          # Écran OLED SSD1306
│   ├── lora_driver.h/.cpp      # Module LoRa SX1278
│   ├── audio.h/.cpp            # Micro + HP + ADPCM
│   └── power_manager.h/.cpp    # Gestion batterie/énergie
│   ├── call_manager.h/.cpp     # Gestion des appels
│   ├── sms_manager.h/.cpp      # Gestion des SMS
│   ├── routing.h/.cpp          # Routage réseau
│   └── crypto.h/.cpp           # Chiffrement AES-128
│   ├── home_screen.h/.cpp      # Écran d'accueil
│   ├── dialer_screen.h/.cpp    # Composeur numérique
│   ├── call_screen.h/.cpp      # Appel en cours
│   ├── contacts_screen.h/.cpp  # Répertoire
│   ├── sms_screen.h/.cpp       # Messages
│   └── settings_screen.h/.cpp  # Paramètres
│   ├── status_bar.h/.cpp       # Barre d'état
│   ├── menu.h/.cpp             # Menu générique
│   ├── dialog.h/.cpp           # Boîtes de dialogue
│   ├── input_field.h/.cpp      # Champ de saisie
│   └── icons.h/.cpp            # Bibliothèque d'icônes
│   ├── buffers.h/.cpp          # Buffers circulaires
│   ├── timers.h/.cpp           # Timers logiciels
│   ├── compression.h/.cpp      # Compression ADPCM
│   ├── storage.h/.cpp          # Stockage Flash/EEPROM
│   └── crc.h/.cpp              # Checksums
│   ├── ARCHITECTURE.md         # Architecture détaillée
│   ├── PROTOCOL.md             # Protocole de communication
│   └── SCHEMATIC.md            # Schéma électronique
└── 📄 LICENSE                  # Licence MIT
```

---

## 📊 Performances

| Métrique | Valeur |
|----------|--------|
| Portée voix | 5 km (SF7/BW250) |
| Portée SMS | 15 km (SF9/BW125) |
| Latence voix | ~80ms |
| Autonomie | 2-3 jours (usage normal) |
| Consommation active | 120mA |
| Consommation veille | 15mA |
| Consommation veille profonde | 2mA |
| Flash utilisée | ~52 KB / 64 KB |
| RAM utilisée | ~12 KB / 20 KB |

---

## 📚 Documentation

- 📐 [Architecture du projet](docs/ARCHITECTURE.md)
- 📡 [Protocole de communication](docs/PROTOCOL.md)
- 🔧 [Schéma électronique](docs/SCHEMATIC.md)

---

## 🤝 Contribuer

Les contributions sont les bienvenues !

1. **Fork** le projet
2. Créer une branche (`git checkout -b feature/ma-fonctionnalite`)
3. Committer (`git commit -m 'Ajout de ma fonctionnalité'`)
4. Pousser (`git push origin feature/ma-fonctionnalite`)
5. Ouvrir une **Pull Request**

---

## 📝 Licence

Ce projet est sous licence **MIT** - voir le fichier [LICENSE](LICENSE) pour plus de détails.

---

## 👤 Auteur

**kopi3daniel (Daniel)**

- GitHub : [@kopi3daniel](https://github.com/kopi3daniel)

---

## ⭐ Remerciements

- Bibliothèque **Keypad** (Mark Stanley, Alexander Brevig)
- Bibliothèques **Adafruit SSD1306 / GFX** (Adafruit)
- **STM32 Core for Arduino** (STMicroelectronics)
- Communauté **Open Source**

---

<div align="center">

**⭐ Si ce projet vous plaît, n'hésitez pas à le star ! ⭐**

</div>
```

