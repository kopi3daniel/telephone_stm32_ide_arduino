## 📄 FICHIER COMPLET : `docs/SCHEMATIC.md`

```markdown
# 🔧 Schéma Électronique - Téléphone LoRa

> **Version :** 1.0.0  
> **Date :** 2024  
> **Cible :** STM32F103C8T6 + RA-02 (SX1278) + Périphériques

---

## 📋 Table des matières

1. [Vue d'ensemble](#vue-densemble)
2. [Alimentation](#alimentation)
3. [Microcontrôleur STM32](#microcontrôleur-stm32)
4. [Module LoRa RA-02](#module-lora-ra-02)
5. [Écran OLED 1.3"](#écran-oled-13)
6. [Clavier matriciel 4×6](#clavier-matriciel-4x6)
7. [Audio (Micro + Haut-parleur)](#audio-micro--haut-parleur)
8. [Boutons et LED](#boutons-et-led)
9. [Buzzer et Vibreur](#buzzer-et-vibreur)
10. [Liste des composants](#liste-des-composants)
11. [Brochage complet](#brochage-complet)
12. [Conseils de câblage](#conseils-de-câblage)
13. [Dépannage](#dépannage)

---

## Vue d'ensemble

### Diagramme de connexion général

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          TÉLÉPHONE LORA - SCHÉMA GÉNÉRAL                     │
│                                                                              │
│                              ┌──────────────────┐                           │
│                              │   BOUTONS         │                           │
│                              │ PB8 = Appel (VERT)│                           │
│                              │ PB9 = Fin (ROUGE) │                           │
│                              └────────┬─────────┘                           │
│                                       │                                      │
│  ┌─────────────┐    ┌────────────────┼────────────────┐    ┌─────────────┐  │
│  │  BATTERIE   │    │                │                │    │   BUZZER    │  │
│  │  3.7V Li-Ion│    │        ┌───────┴───────┐        │    │   PC15      │  │
│  │     │       │    │        │  STM32F103C8T6 │        │    │             │  │
│  │  ┌──┴──┐    │    │        │   (Blue Pill)  │        │    └─────────────┘  │
│  │  │ ADC │────┼────┼────────┤ PB0 (Batterie) │        │                     │
│  │  │ PB0 │    │    │        │                │        │    ┌─────────────┐  │
│  │  └─────┘    │    │        │ SPI1 :         │        │    │  VIBREUR    │  │
│  └─────────────┘    │        │ PA4=NSS ───────┼────────┼────│  PC14       │  │
│                      │        │ PA5=SCK ───────┼────────┼────│             │  │
│  ┌─────────────┐    │        │ PA6=MISO ──────┼────────┼────│             │  │
│  │  SX1278     │    │        │ PA7=MOSI ──────┼────────┼────│             │  │
│  │  RA-02      │    │        │ PA0=DIO0 ──────┼────────┼────│             │  │
│  │  (LoRa)     │    │        │ PA2=RST ───────┼────────┼────│             │  │
│  │             │    │        │                │        │    └─────────────┘  │
│  │  868 MHz    │    │        │ I2C1 :         │        │                     │
│  │  Antenne    │    │        │ PB6=SCL ───────┼────────┼────┐               │
│  └─────────────┘    │        │ PB7=SDA ───────┼────────┼────┼─┐            │
│                      │        │                │        │    │ │            │
│  ┌─────────────┐    │        │ ADC1 :         │        │    │ │            │
│  │ MICROPHONE  │    │        │ PA1 ───────────┼────────┼──┐ │ │            │
│  │ + Préampli  │────┼────────┤                │        │  │ │ │            │
│  │             │    │        │ TIM1 PWM :     │        │  │ │ │            │
│  └─────────────┘    │        │ PA8 ───────────┼────────┼──┼─┼─┼─┐          │
│                      │        │                │        │  │ │ │ │          │
│  ┌─────────────┐    │        │ GPIO :         │        │  │ │ │ │          │
│  │ HAUT-PARLEUR│    │        │ PC13 ──────────┼────────┼──┼─┼─┼─┼─┐        │
│  │ + Ampli     │────┼────────┤ (LED Torche)   │        │  │ │ │ │ │        │
│  │             │    │        │                │        │  │ │ │ │ │        │
│  └─────────────┘    │        │ Clavier :      │        │  │ │ │ │ │        │
│                      │        │ PB12-15 ───────┼────────┼──┼─┼─┼─┼─┼──┐    │
│  ┌─────────────┐    │        │ PA8-PA9 ───────┼────────┼──┼─┼─┼─┼─┼──┼─┐  │
│  │  OLED 1.3"  │    │        │ PB3-PB5 ───────┼────────┼──┼─┼─┼─┼─┼──┼─┼┐ │
│  │  SSD1306    │────┼────────┤ PA10 ──────────┼────────┼──┼─┼─┼─┼─┼──┼─┼┤ │
│  │  128×64     │    │        │                │        │  │ │ │ │ │  │ ││ │
│  └─────────────┘    │        └────────────────┘        │  │ │ │ │ │  │ ││ │
│                      │                                  │  │ │ │ │ │  │ ││ │
│                      │    ┌──────────────────┐         │  │ │ │ │ │  │ ││ │
│                      │    │    CLAVIER 4×6   │         │  │ │ │ │ │  │ ││ │
│                      │    │ 24 touches       │         │  │ │ │ │ │  │ ││ │
│                      │    └──────────────────┘         │  │ │ │ │ │  │ ││ │
│                      │                                  │  │ │ │ │ │  │ ││ │
└──────────────────────────────────────────────────────────┘  │ │ │ │ │  │ ││ │
                                                              │ │ │ │ │  │ ││ │
  LÉGENDE :                                                   │ │ │ │ │  │ ││ │
  ────── Connexion SPI (4 fils)                              │ │ │ │ │  │ ││ │
  ────── Connexion I2C (2 fils)                              │ │ │ │ │  │ ││ │
  ────── Connexion Analogique (ADC/PWM)                     │ │ │ │ │  │ ││ │
  ────── Connexion GPIO simple                               │ │ │ │ │  │ ││ │
```

---

## Alimentation

### Schéma d'alimentation

```
┌─────────────────────────────────────────────────────────────┐
│                      ALIMENTATION                            │
│                                                              │
│  Batterie Li-Ion    Régulateur 3.3V                         │
│  3.7V 2000mAh       AMS1117-3.3                             │
│       │                  │                                   │
│  ┌────┴────┐        ┌────┴────┐                             │
│  │   +     │────────│   IN    │──── 3.3V ───► Tous les     │
│  │         │        │         │               composants    │
│  │  BATT   │   ┌────┤  OUT    │                             │
│  │         │   │    │         │─── GND ───► Masse commune   │
│  │   -     │───┤    └─────────┘                             │
│  └─────────┘   │                                            │
│                │    ┌─────────────┐                         │
│                │    │ Diviseur    │                         │
│                └────│ 10kΩ + 10kΩ │─── PB0 (ADC) ──► Mesure│
│                     └─────────────┘              batterie   │
│                                                              │
│  Condensateurs de découplage :                              │
│  - 100µF électrolytique (entrée)                            │
│  - 10µF céramique (sortie)                                  │
│  - 100nF sur chaque VDD du STM32                            │
│  - 100nF sur VCC du SX1278                                  │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### Points importants

| Élément | Valeur | Remarque |
|---------|--------|----------|
| Tension batterie | 3.7V nominal (4.2V max) | Li-Ion 18650 ou Li-Po |
| Régulateur | AMS1117-3.3V | 800mA max |
| Consommation max | ~300mA | En émission LoRa + audio |
| Autonomie estimée | 2-3 jours | Usage normal |
| Protection | TP4056 | Module chargeur USB |

---

## Microcontrôleur STM32

### STM32F103C8T6 (Blue Pill)

```
                    ┌──────────────┐
                    │ STM32F103C8T6│
                    │   (Blue Pill)│
                    │              │
    Alimentation ───┤VDD        GND├─── Masse
                    │              │
    Debug SWD    ───┤PA13       PB0├─── Mesure batterie
    Debug SWC    ───┤PA14       PB1├─── (libre)
                    │              │
    LoRa DIO0    ───┤PA0        PB3├─── Clavier Col 1
    Micro ADC    ───┤PA1        PB4├─── Clavier Col 2
    LoRa RST     ───┤PA2        PB5├─── Clavier Col 3
    Volume +     ───┤PA3        PB6├─── OLED SCL
    LoRa NSS     ───┤PA4        PB7├─── OLED SDA
    LoRa SCK     ───┤PA5        PB8├─── Bouton APPEL
    LoRa MISO    ───┤PA6        PB9├─── Bouton FIN
    LoRa MOSI    ───┤PA7       PB10├─── UART TX (Debug)
    HP PWM       ───┤PA8       PB11├─── UART RX (Debug)
    Clavier L5   ───┤PA9       PB12├─── Clavier Ligne 1
    Clavier C4   ───┤PA10      PB13├─── Clavier Ligne 2
    USB DM       ───┤PA11      PB14├─── Clavier Ligne 3
    USB DP       ───┤PA12      PB15├─── Clavier Ligne 4
                    │              │
    LED Torche   ───┤PC13      PC14├─── Vibreur
                    │         PC15├─── Buzzer
                    └──────────────┘
```

---

## Module LoRa RA-02

### Connexions SX1278

```
┌─────────────────────────────────────────────┐
│           RA-02 (SX1278)                     │
│                                              │
│  ┌──────────┐                               │
│  │  Antenne │─── Fil quart d'onde            │
│  │  SMA     │    (8.6 cm pour 868 MHz)      │
│  └──────────┘                               │
│                                              │
│  Brochage :                                  │
│  ┌─────────┬────────────────────────────┐   │
│  │ VCC     │ 3.3V (⚠️ PAS 5V !)         │   │
│  │ GND     │ Masse                       │   │
│  │ NSS     │ PA4  (SPI Chip Select)      │   │
│  │ SCK     │ PA5  (SPI Clock)            │   │
│  │ MOSI    │ PA7  (SPI Master Out)       │   │
│  │ MISO    │ PA6  (SPI Master In)        │   │
│  │ DIO0    │ PA0  (Interruption RX done) │   │
│  │ DIO1    │ NC   (Non connecté)         │   │
│  │ DIO2    │ NC                          │   │
│  │ DIO3    │ NC                          │   │
│  │ DIO4    │ NC                          │   │
│  │ DIO5    │ NC                          │   │
│  │ RESET   │ PA2  (Reset matériel)       │   │
│  └─────────┴────────────────────────────┘   │
│                                              │
│  ⚠️  IMPORTANT :                             │
│  - Alimentation 3.3V UNIQUEMENT              │
│  - Antenne obligatoire pour émettre          │
│  - Découplage 100nF + 10µF sur VCC          │
│  - Ne pas émettre sans antenne !             │
└─────────────────────────────────────────────┘
```

### Antenne

| Fréquence | Longueur d'onde | Quart d'onde | Type recommandé |
|-----------|-----------------|--------------|-----------------|
| 868 MHz | 34.5 cm | 8.6 cm | Fil droit ou hélicoïdale |
| 915 MHz | 32.8 cm | 8.2 cm | Fil droit ou hélicoïdale |
| 433 MHz | 69.3 cm | 17.3 cm | Fil droit |

---

## Écran OLED 1.3"

### Connexions SSD1306

```
┌─────────────────────────────────────────────┐
│         OLED 1.3" SSD1306 128×64             │
│                                              │
│  Brochage :                                  │
│  ┌─────────┬────────────────────────────┐   │
│  │ VCC     │ 3.3V                       │   │
│  │ GND     │ Masse                      │   │
│  │ SCL     │ PB6 (I2C1 Clock)           │   │
│  │ SDA     │ PB7 (I2C1 Data)            │   │
│  └─────────┴────────────────────────────┘   │
│                                              │
│  Adresse I2C : 0x3C                          │
│  Résolution   : 128 × 64 pixels              │
│  Couleur      : Monochrome (blanc/bleu)      │
│  Consommation : ~20mA (allumé)               │
│                                              │
│  Pull-up I2C :                               │
│  - 4.7kΩ entre SCL et 3.3V                  │
│  - 4.7kΩ entre SDA et 3.3V                  │
│  (souvent déjà présents sur le module)       │
└─────────────────────────────────────────────┘
```

---

## Clavier matriciel 4×6

### Schéma de la matrice

```
┌─────────────────────────────────────────────────────────────┐
│                    CLAVIER MATRICIEL 4×6                     │
│                                                              │
│  Organisation : 6 LIGNES × 4 COLONNES = 24 touches          │
│                                                              │
│  COLONNES (Entrées STM32 avec pull-up) :                    │
│  Col 1 ─── PB3                                              │
│  Col 2 ─── PB4                                              │
│  Col 3 ─── PB5                                              │
│  Col 4 ─── PA10                                             │
│                                                              │
│  LIGNES (Sorties STM32) :                                   │
│  Ligne 1 ─── PB12                                           │
│  Ligne 2 ─── PB13                                           │
│  Ligne 3 ─── PB14                                           │
│  Ligne 4 ─── PB15                                           │
│  Ligne 5 ─── PA9                                            │
│  Ligne 6 ─── PA8 (⚠️ partagé avec PWM audio !)              │
│                                                              │
│  ⚠️  CONFLIT : PA8 est utilisé pour le HP (TIM1_CH1)        │
│      et la ligne 6 du clavier.                               │
│      SOLUTION : Utiliser une diode pour isoler               │
│      ou déplacer la ligne 6 sur une autre pin libre.         │
│                                                              │
│  Matrice de connexion :                                     │
│                                                              │
│       Col1   Col2   Col3   Col4                             │
│       PB3    PB4    PB5    PA10                             │
│  ┌─────┬─────┬─────┬─────┐                                  │
│  │ '1' │ '2' │ '3' │ 'A' │── Ligne 1 (PB12)                │
│  ├─────┼─────┼─────┼─────┤                                  │
│  │ '4' │ '5' │ '6' │ 'B' │── Ligne 2 (PB13)                │
│  ├─────┼─────┼─────┼─────┤                                  │
│  │ '7' │ '8' │ '9' │ 'C' │── Ligne 3 (PB14)                │
│  ├─────┼─────┼─────┼─────┤                                  │
│  │ '*' │ '0' │ '#' │ 'D' │── Ligne 4 (PB15)                │
│  ├─────┼─────┼─────┼─────┤                                  │
│  │ 'E' │ 'F' │ 'G' │ 'H' │── Ligne 5 (PA9)                 │
│  ├─────┼─────┼─────┼─────┤                                  │
│  │ 'I' │ 'J' │ 'K' │ 'L' │── Ligne 6 (PA8) ⚠️               │
│  └─────┴─────┴─────┴─────┘                                  │
│                                                              │
│  Symbole des touches :                                      │
│  A = APPEL (vert)     C = Contacts                          │
│  B = RETOUR (rouge)   D = Messages                          │
│  I = Shift            J = Mode (abc/ABC/123)                │
│  K = Torche           L = Verrouiller                       │
│  E = Volume +         F = Volume -                          │
│  G = Menu             H = Options                           │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### Correction du conflit PA8

```
Option A : Diode d'isolement
─────────────────────────────
PA8 ────│>├──── Ligne 6 du clavier
         │
         └──── PWM Audio (via résistance)

La diode empêche le signal PWM d'activer la ligne du clavier.

Option B : Déplacer la ligne 6 (recommandé)
─────────────────────────────────────────────
Utiliser une pin libre à la place de PA8 :
- PA15 (si JTAG désactivé)
- PB1 (si non utilisée)
- PA11 ou PA12 (si USB non utilisé)
```

---

## Audio (Micro + Haut-parleur)

### Circuit du microphone

```
┌─────────────────────────────────────────────────────────────┐
│                    CIRCUIT MICROPHONE                        │
│                                                              │
│  Micro Électret ────│├────┬─────────── PA1 (ADC1_CH1)      │
│                     C1    │                                  │
│                    100nF  │                                  │
│                           ├──── R2 ──── 3.3V               │
│                           │     10kΩ                        │
│                           │                                  │
│                          R1                                  │
│                          10kΩ                               │
│                           │                                  │
│                          GND                                 │
│                                                              │
│  Composants :                                               │
│  - MIC : Microphone électret standard                        │
│  - C1  : 100nF (condensateur de liaison)                    │
│  - R1  : 10kΩ (polarisation microphone)                     │
│  - R2  : 10kΩ (diviseur de tension)                         │
│                                                              │
│  Préamplificateur (optionnel, recommandé) :                 │
│  Utiliser un transistor NPN (2N2222) ou AOP (LM358)         │
│  Gain : ×10 à ×50                                           │
│                                                              │
│  Préampli simple avec transistor :                          │
│                                                              │
│                     3.3V                                     │
│                      │                                       │
│                     R3 10kΩ                                  │
│                      │                                       │
│  Micro ──C1──┬───────B 2N2222                               │
│              │       E──┴───R4───GND                        │
│             R1          100Ω                                │
│              │       C───┴───┬─── PA1 (ADC)                  │
│             GND             C2 10µF                          │
│                              │                               │
│                             GND                              │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### Circuit du haut-parleur

```
┌─────────────────────────────────────────────────────────────┐
│                  CIRCUIT HAUT-PARLEUR                        │
│                                                              │
│  PA8 (TIM1_CH1 PWM) ──R1──┬────────── HP 8Ω               │
│                    1kΩ    │                                  │
│                           ├── C1 ──── GND                   │
│                           │   100nF                         │
│                           │                                  │
│  Filtre RC passe-bas pour lisser la PWM :                   │
│  Fréquence de coupure = 1/(2π×R1×C1)                        │
│                     = 1/(2×3.14×1000×0.0000001)             │
│                     ≈ 1590 Hz                                │
│                                                              │
│  ┌──────────────────────────────────────────┐              │
│  │  AMPLIFICATEUR AUDIO LM386 (recommandé)   │              │
│  │                                           │              │
│  │  PA8 ──R1──┬───── IN+    OUT ──C2──┬── HP │              │
│  │      1kΩ   │                 100µF  │  8Ω │              │
│  │           C1 100nF                  │      │              │
│  │            │                        │      │              │
│  │           GND                      GND     │              │
│  │                                           │              │
│  │  3.3V ──── VCC                           │              │
│  │  GND ──── GND                            │              │
│  │  Gain = 20 (pin 1-8 ouvert)              │              │
│  │  Gain = 200 (pin 1-8 avec 10µF)          │              │
│  └──────────────────────────────────────────┘              │
│                                                              │
│  ⚠️  La PWM directe du STM32 ne peut pas alimenter          │
│      un haut-parleur directement !                           │
│      Utilisez OBLIGATOIREMENT un amplificateur.              │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## Boutons et LED

### Boutons physiques

```
┌─────────────────────────────────────────────┐
│         BOUTONS ET LED TORCHE               │
│                                              │
│  Bouton APPEL (VERT) :                       │
│  3.3V ──── R1 10kΩ ────┬──── PB8            │
│                         │                    │
│                    ┌────┴────┐              │
│                    │ Bouton  │               │
│                    │ Poussoir │              │
│                    └────┬────┘              │
│                         │                    │
│                        GND                   │
│                                              │
│  Bouton FIN (ROUGE) :                       │
│  3.3V ──── R2 10kΩ ────┬──── PB9            │
│                         │                    │
│                    ┌────┴────┐              │
│                    │ Bouton  │               │
│                    │ Poussoir │              │
│                    └────┬────┘              │
│                         │                    │
│                        GND                   │
│                                              │
│  LED Torche (Haute luminosité) :             │
│  PC13 ──── R3 220Ω ──── LED ──── GND        │
│                                              │
│  ⚠️  La LED ne doit pas dépasser 20mA        │
│      R = (3.3V - 2V) / 0.02A = 65Ω          │
│      Utiliser 100Ω pour sécurité             │
└─────────────────────────────────────────────┘
```

---

## Buzzer et Vibreur

```
┌─────────────────────────────────────────────┐
│            BUZZER ET VIBREUR                 │
│                                              │
│  Buzzer piézoélectrique :                    │
│  PC15 ──── Transistor ──── Buzzer ──── 3.3V│
│           (2N2222)                           │
│                                              │
│  Protection : Diode flyback 1N4148          │
│                                              │
│  Vibreur (moteur vibrant) :                  │
│  PC14 ──── Transistor ──── Moteur ──── 3.3V│
│           (2N2222)                           │
│                                              │
│  ⚠️  Ne pas connecter directement aux GPIO   │
│      Utiliser un transistor pour commuter    │
│                                              │
│  Circuit type :                              │
│  GPIO ── R1 1kΩ ── B (Transistor)           │
│                      C ──── Charge ──── VCC  │
│                      E ──── GND               │
│                                              │
└─────────────────────────────────────────────┘
```

---

## Liste des composants

### Composants principaux

| # | Composant | Valeur/Modèle | Quantité | Prix approx. |
|---|-----------|---------------|----------|-------------|
| 1 | Microcontrôleur | STM32F103C8T6 (Blue Pill) | 1 | ~3€ |
| 2 | Module LoRa | RA-02 (SX1278) 868MHz | 1 | ~8€ |
| 3 | Écran OLED | 1.3" SSD1306 128×64 I2C | 1 | ~4€ |
| 4 | Régulateur | AMS1117-3.3V | 1 | ~0.5€ |
| 5 | Chargeur batterie | TP4056 micro-USB | 1 | ~1€ |
| 6 | Batterie | Li-Ion 18650 2000mAh | 1 | ~5€ |
| 7 | Ampli audio | LM386 module | 1 | ~2€ |
| 8 | Haut-parleur | 8Ω 0.5W | 1 | ~1€ |
| 9 | Microphone | Électret | 1 | ~0.5€ |

### Composants passifs

| # | Composant | Valeur | Quantité |
|---|-----------|--------|----------|
| 10 | Résistance | 10kΩ | 10 |
| 11 | Résistance | 1kΩ | 5 |
| 12 | Résistance | 220Ω | 3 |
| 13 | Résistance | 100Ω | 2 |
| 14 | Condensateur | 100µF électro. | 3 |
| 15 | Condensateur | 10µF | 3 |
| 16 | Condensateur | 100nF céramique | 10 |
| 17 | Condensateur | 10nF | 2 |
| 18 | LED | 5mm blanche (torche) | 1 |
| 19 | LED | 3mm verte (appel) | 1 |
| 20 | LED | 3mm rouge (fin) | 1 |

### Divers

| # | Composant | Valeur/Modèle | Quantité |
|---|-----------|---------------|----------|
| 21 | Bouton poussoir | 6×6mm | 26 |
| 22 | Buzzer | Piézo 5V | 1 |
| 23 | Moteur vibrant | Mini vibreur | 1 |
| 24 | Transistor NPN | 2N2222 | 3 |
| 25 | Diode | 1N4148 | 2 |
| 26 | Antenne | 868MHz SMA | 1 |
| 27 | Support batterie | 18650 | 1 |
| 28 | Câbles Dupont | Mâle-Femelle | ~30 |

### Coût total estimé : ~35-40€

---

## Brochage complet

### Tableau récapitulatif

```
┌────────┬────────────────────────────────┬──────────┐
│ Pin    │ Fonction                       │ Conflit  │
├────────┼────────────────────────────────┼──────────┤
│ PA0    │ LoRa DIO0 (Interruption)       │          │
│ PA1    │ Microphone (ADC1_CH1)          │          │
│ PA2    │ LoRa RESET                     │          │
│ PA3    │ (libre pour Volume+)           │          │
│ PA4    │ LoRa NSS (SPI1)                │          │
│ PA5    │ LoRa SCK (SPI1)                │          │
│ PA6    │ LoRa MISO (SPI1)               │          │
│ PA7    │ LoRa MOSI (SPI1)               │          │
│ PA8    │ HP PWM (TIM1_CH1)              │⚠️ Clavier│
│ PA9    │ Clavier Ligne 5                │          │
│ PA10   │ Clavier Colonne 4              │          │
│ PA11   │ (libre / USB_DM)               │          │
│ PA12   │ (libre / USB_DP)               │          │
│ PA13   │ SWDIO (Debug)                  │          │
│ PA14   │ SWCLK (Debug)                  │          │
│ PA15   │ (libre si JTAG désactivé)      │          │
├────────┼────────────────────────────────┼──────────┤
│ PB0    │ Mesure batterie (ADC1_CH8)     │          │
│ PB1    │ (libre)                        │          │
│ PB3    │ Clavier Colonne 1              │          │
│ PB4    │ Clavier Colonne 2              │          │
│ PB5    │ Clavier Colonne 3              │          │
│ PB6    │ OLED SCL (I2C1)                │          │
│ PB7    │ OLED SDA (I2C1)                │          │
│ PB8    │ Bouton APPEL (VERT)            │          │
│ PB9    │ Bouton FIN (ROUGE)            │          │
│ PB10   │ UART3 TX (Debug)               │          │
│ PB11   │ UART3 RX (Debug)               │          │
│ PB12   │ Clavier Ligne 1               │          │
│ PB13   │ Clavier Ligne 2               │          │
│ PB14   │ Clavier Ligne 3               │          │
│ PB15   │ Clavier Ligne 4               │          │
├────────┼────────────────────────────────┼──────────┤
│ PC13   │ LED Torche                     │          │
│ PC14   │ Vibreur                        │          │
│ PC15   │ Buzzer                         │          │
└────────┴────────────────────────────────┴──────────┘
```

---

## Conseils de câblage

### 🔧 Bonnes pratiques

1. **Alimentation**
   - Placer les condensateurs de découplage au plus près des composants
   - Utiliser des fils d'alimentation suffisamment épais (≥0.5mm²)
   - Séparer la masse de puissance (LoRa TX) de la masse de signal

2. **SPI (LoRa)**
   - Garder les pistes SPI courtes (< 5cm)
   - Si possible, torsader SCK avec GND
   - Ne pas croiser les pistes SPI avec des signaux de puissance

3. **I2C (OLED)**
   - Résistances de pull-up 4.7kΩ sur SDA et SCL
   - Garder les pistes I2C ensemble
   - Éviter les nappes trop longues (> 20cm)

4. **Audio**
   - Séparer la masse analogique (micro, HP) de la masse numérique
   - Utiliser un plan de masse pour l'amplificateur
   - Blindage du câble microphone

5. **Antenne**
   - Garder l'antenne dégagée (pas de masse à proximité)
   - Éviter les boucles de masse près de l'antenne
   - Ne pas toucher l'antenne pendant l'émission

### ⚠️ Points critiques

- **PA8 partagé** : Résoudre le conflit clavier/PWM (diode ou pin différente)
- **3.3V uniquement** : Le SX1278 et l'OLED ne supportent PAS 5V
- **Antenne obligatoire** : Ne jamais émettre sans antenne (risque de dommage)
- **Découplage** : Condensateur 100nF sur CHAQUE VCC

---

## Dépannage

### Problèmes courants

| Problème | Cause probable | Solution |
|----------|---------------|----------|
| Le STM32 ne démarre pas | Alimentation instable | Vérifier le régulateur 3.3V |
| LoRa ne répond pas | SPI mal câblé | Vérifier NSS, SCK, MOSI, MISO |
| OLED n'affiche rien | I2C non connecté | Vérifier SDA, SCL, pull-ups |
| Pas de son | Ampli non alimenté | Vérifier l'ampli et le HP |
| Micro ne capte rien | Préampli absent | Ajouter un préamplificateur |
| Portée très faible | Antenne absente/mal | Vérifier l'antenne 868 MHz |
| Consommation excessive | Court-circuit | Vérifier les soudures |
| Clavier fantôme | Lignes/colonnes inversées | Vérifier le brochage |
| Larsen (sifflement) | Micro trop près du HP | Éloigner ou baisser le volume |
| Batterie ne charge pas | TP4056 mal câblé | Vérifier le module de charge |

---

## 📚 Références

- [Architecture du projet](ARCHITECTURE.md)
- [Protocole de communication](PROTOCOL.md)
- [Manuel utilisateur](USER_MANUAL.md)
- [Datasheet STM32F103C8T6](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- [Datasheet SX1278](https://www.semtech.com/products/wireless-rf/lora-connect/sx1278)
- [Datasheet SSD1306](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
```

---

