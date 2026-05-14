## 📄 FICHIER COMPLET : `docs/USER_MANUAL.md`

```markdown
# 📖 Manuel Utilisateur - Téléphone LoRa

> **Version :** 1.0.0  
> **Date :** 2024  
> **Appareil :** Téléphone LoRa

---

## 📋 Table des matières

1. [Présentation](#présentation)
2. [Première utilisation](#première-utilisation)
3. [Description de l'appareil](#description-de-lappareil)
4. [Écran d'accueil](#écran-daccueil)
5. [Passer un appel](#passer-un-appel)
6. [Recevoir un appel](#recevoir-un-appel)
7. [Pendant un appel](#pendant-un-appel)
8. [Envoyer un SMS](#envoyer-un-sms)
9. [Lire les messages](#lire-les-messages)
10. [Gérer les contacts](#gérer-les-contacts)
11. [Journal d'appels](#journal-dappels)
12. [Paramètres](#paramètres)
13. [Saisie de texte (Multi-tap)](#saisie-de-texte-multi-tap)
14. [Batterie et énergie](#batterie-et-énergie)
15. [Sécurité](#sécurité)
16. [Dépannage](#dépannage)
17. [Foire aux questions](#foire-aux-questions)

---

## Présentation

### Qu'est-ce que le Téléphone LoRa ?

Le Téléphone LoRa est un appareil de communication qui fonctionne **sans réseau mobile**. Il utilise la technologie radio LoRa pour transmettre la voix et les messages texte sur de longues distances (5 à 15 km).

**Pas besoin de :**
- ❌ Carte SIM
- ❌ Abonnement téléphonique
- ❌ Antenne relais
- ❌ WiFi
- ❌ Bluetooth

**Idéal pour :**
- ✅ Zones rurales sans couverture mobile
- ✅ Communications de groupe (plusieurs téléphones)
- ✅ Situations d'urgence
- ✅ Activités outdoor (randonnée, camping)
- ✅ Projets communautaires

### Contenu de la boîte

| Élément | Description |
|---------|-------------|
| Téléphone LoRa | L'appareil principal |
| Antenne 868 MHz | Antenne amovible |
| Batterie | Li-Ion 18650 2000mAh |
| Câble USB | Pour le chargement |
| Guide de démarrage | Ce document |

---

## Première utilisation

### 1. Insérer la batterie

```
1. Ouvrir le compartiment batterie (à l'arrière)
2. Insérer la batterie 18650 (respecter la polarité + / -)
3. Refermer le compartiment
```

### 2. Visser l'antenne

```
1. Prendre l'antenne fournie
2. La visser sur le connecteur SMA en haut de l'appareil
3. Ne pas forcer, visser à la main
⚠️ Ne jamais allumer le téléphone sans antenne !
```

### 3. Charger la batterie

```
1. Brancher le câble USB sur le port de charge
2. Connecter à une source USB (PC, chargeur secteur)
3. La LED rouge s'allume pendant la charge
4. La LED verte s'allume quand la charge est terminée
5. Première charge : laisser charger 4 heures
```

### 4. Allumer le téléphone

```
1. Maintenir le bouton POWER enfoncé 3 secondes
2. L'écran affiche le logo "PHONE LORA"
3. Après 2 secondes, l'écran d'accueil apparaît
4. Le téléphone est prêt !
```

### 5. Configurer votre numéro

```
Par défaut, votre téléphone a le numéro "0001".

Pour le changer :
1. Aller dans Menu → Réglages → Système
2. Sélectionner "Identité"
3. Modifier le numéro (4 chiffres hexadécimaux)
4. Appuyer sur VERT pour enregistrer

Exemples de numéros : 0001, 0002, A0F3, BEEF
```

---

## Description de l'appareil

### Vue de face

```
┌──────────────────────────────────────────────┐
│  ┌──────────────────────────────────────┐    │
│  │          ÉCRAN OLED 1.3"             │    │
│  │         (barre d'état en haut)       │    │
│  │                                      │    │
│  │     Affiche l'heure, les menus,      │    │
│  │     les messages, les appels...      │    │
│  │                                      │    │
│  └──────────────────────────────────────┘    │
│                                              │
│  ┌───┬───┬───┬───┐                           │
│  │ 1 │ 2 │ 3 │ A │  ← Bouton APPEL (VERT)    │
│  ├───┼───┼───┼───┤                           │
│  │ 4 │ 5 │ 6 │ B │  ← Bouton RETOUR (ROUGE)  │
│  ├───┼───┼───┼───┤                           │
│  │ 7 │ 8 │ 9 │ C │  ← Contacts               │
│  ├───┼───┼───┼───┤                           │
│  │ * │ 0 │ # │ D │  ← Messages               │
│  ├───┼───┼───┼───┤                           │
│  │ E │ F │ G │ H │  ← Fonctions              │
│  ├───┼───┼───┼───┤                           │
│  │ I │ J │ K │ L │  ← Shift, Mode, Torche... │
│  └───┴───┴───┴───┘                           │
│                                              │
│   Microphone (en bas)                        │
└──────────────────────────────────────────────┘
```

### Vue de profil

```
┌──────────────────────────────────────────────┐
│                    ANTENNE                   │
│                       │                      │
│  ┌────────────────────┼──────────────────┐   │
│  │                    │                  │   │
│  │  [BOUTON VERT]    [BOUTON ROUGE]     │   │
│  │  (Appel)          (Raccrocher)       │   │
│  │                                      │   │
│  │  [BOUTON POWER]   [PORT USB]         │   │
│  │  (Allumer/Éteindre) (Charge)         │   │
│  │                                      │   │
│  │  [LED TORCHE]                        │   │
│  │                                      │   │
│  └──────────────────────────────────────┘   │
│                    │                         │
│              HAUT-PARLEUR                    │
└──────────────────────────────────────────────┘
```

### Signification des touches

| Touche | Nom | Fonction |
|--------|-----|----------|
| **0-9** | Numériques | Composer un numéro, saisie texte |
| **\*** | Étoile | Ponctuation, mode symbole |
| **#** | Dièse | Changer mode (abc/ABC/123) |
| **A** | Appel (VERT) | Lancer un appel, décrocher, envoyer |
| **B** | Retour (ROUGE) | Raccrocher, annuler, retour |
| **C** | Contacts | Ouvrir le répertoire |
| **D** | Messages | Ouvrir les SMS |
| **E** | Volume + | Augmenter le volume |
| **F** | Volume - | Diminuer le volume |
| **G** | Menu | Ouvrir le menu contextuel |
| **H** | Options | Options supplémentaires |
| **I** | Shift | Majuscule (Caps Lock) |
| **J** | Mode | Changer mode abc/ABC/123 |
| **K** | Torche | Allumer/éteindre la lampe |
| **L** | Verrouiller | Verrouiller le téléphone |

---

## Écran d'accueil

### Description

```
┌──────────────────────────┐
│ 📶 -85dBm   14:30  🔋85%│ ← Barre d'état
├──────────────────────────┤
│                          │
│        14:30             │ ← Horloge
│      Lun 15 Jan          │ ← Date
│                          │
│  [📞]  [✉️]  [📇]  [⚙️]   │ ← Menu principal
│  Appels SMS  Cts  Régl.  │
│                          │
└──────────────────────────┘
```

### Barre d'état

| Icône | Signification |
|-------|---------------|
| 📶 | Force du signal LoRa (0-4 barres) |
| -85dBm | Valeur RSSI en dBm |
| 14:30 | Heure actuelle |
| 🔋85% | Niveau de batterie |
| ✉️ | Nouveau message non lu |
| 📞 | Appel manqué |
| 🔇 | Mode silencieux activé |
| 🔦 | Torche allumée |
| 🔒 | Téléphone verrouillé |

---

## Passer un appel

### Étape par étape

1. **Depuis l'écran d'accueil :**
   - Composer le numéro directement avec les touches 0-9
   - Ou appuyer sur `C` pour choisir dans les contacts

2. **Vérifier le numéro :**
   - Le numéro s'affiche à l'écran
   - Appuyer sur `C` pour effacer un chiffre
   - Appuyer longuement sur `C` pour tout effacer

3. **Lancer l'appel :**
   - Appuyer sur le bouton **VERT (A)**
   - L'écran affiche "Appel en cours..."
   - Vous entendez la tonalité d'attente

4. **Pendant la sonnerie :**
   - Attendre que le correspondant décroche
   - Pour annuler, appuyer sur **ROUGE (B)**

5. **Quand le correspondant décroche :**
   - L'écran affiche la durée de l'appel
   - Parlez normalement dans le micro
   - Le son sort par le haut-parleur

### Appeler depuis les contacts

1. Appuyer sur `C` (Contacts)
2. Naviguer avec `2`/`8` (haut/bas)
3. Appuyer sur **VERT** pour appeler le contact sélectionné

### Recomposer le dernier numéro

1. Depuis l'écran d'accueil
2. Appuyer deux fois sur **VERT**
3. Le dernier numéro est recomposé automatiquement

---

## Recevoir un appel

### Quand quelqu'un vous appelle

1. **Le téléphone sonne :**
   - L'écran affiche "APPEL ENTRANT"
   - Le nom du correspondant (si dans les contacts)
   - Le numéro du correspondant
   - Le vibreur s'active (si activé dans les réglages)

2. **Pour répondre :**
   - Appuyer sur le bouton **VERT (A)**
   - La communication commence

3. **Pour refuser :**
   - Appuyer sur le bouton **ROUGE (B)**
   - L'appelant entendra le signal "occupé"
   - L'appel sera noté comme "manqué"

4. **Pour mettre en silencieux :**
   - Appuyer sur `#` pendant la sonnerie
   - La sonnerie s'arrête mais l'appel continue d'arriver
   - Vous pouvez encore répondre

### Si vous ne répondez pas

- Après 30 secondes, l'appel est automatiquement marqué comme "manqué"
- Une notification apparaît sur l'écran d'accueil
- L'appel est enregistré dans le journal

---

## Pendant un appel

### Options disponibles

```
┌──────────────────────────┐
│   Appel avec BOB         │
│                          │
│       ☎️ 🟢              │
│    00:02:34              │ ← Durée
│                          │
│  [🎤 Muet] [🔊 HP]       │
│  [➕ Vol+]  [❌ Fin]     │
└──────────────────────────┘
```

| Action | Touche | Effet |
|--------|--------|-------|
| **Raccrocher** | ROUGE (B) | Termine l'appel |
| **Muet** | `*` | Coupe votre micro (le correspondant ne vous entend plus) |
| **Haut-parleur** | `0` | Active/désactive le mode mains-libres |
| **Volume +** | `E` | Augmente le volume |
| **Volume -** | `F` | Diminue le volume |

### Qualité audio

- La qualité est comparable à un talkie-walkie
- Pour une meilleure qualité :
  - Parlez à 10-15 cm du micro
  - Parlez distinctement, pas trop vite
  - Évitez le bruit de fond
  - Tournez-vous vers une direction dégagée

---

## Envoyer un SMS

### Écrire un nouveau message

1. **Ouvrir l'éditeur :**
   - Depuis l'accueil, appuyer sur `D` (Messages)
   - Ou Menu → Messages → Nouveau

2. **Entrer le destinataire :**
   - Taper le numéro du destinataire
   - Ou appuyer sur `C` pour choisir dans les contacts

3. **Écrire le message :**
   - Utiliser le clavier multi-tap (voir section "Saisie de texte")
   - Maximum 160 caractères
   - Le compteur affiche les caractères restants

4. **Envoyer :**
   - Appuyer sur **VERT (A)**
   - L'écran affiche "Envoi..."
   - Puis "Message envoyé !"

### Caractères spéciaux

| Touche | Mode abc | Mode ABC | Mode 123 |
|--------|----------|----------|----------|
| 0 | Espace | Espace | 0 |
| 1 | . , ! ? | . , ! ? | 1 |
| 2 | a b c | A B C | 2 |
| 3 | d e f | D E F | 3 |
| 4 | g h i | G H I | 4 |
| 5 | j k l | J K L | 5 |
| 6 | m n o | M N O | 6 |
| 7 | p q r s | P Q R S | 7 |
| 8 | t u v | T U V | 8 |
| 9 | w x y z | W X Y Z | 9 |

---

## Lire les messages

### Accéder aux conversations

1. Appuyer sur `D` (Messages) depuis l'accueil
2. La liste des conversations s'affiche
3. Naviguer avec `2`/`8` (haut/bas)
4. Appuyer sur **VERT** pour ouvrir une conversation

### Dans une conversation

```
┌──────────────────────────┐
│   Conversation avec BOB  │
├──────────────────────────┤
│                          │
│  Moi : Salut !           │
│  Bob : Ça va ?           │
│  Moi : Oui, et toi ?    │
│                          │
│  [VERT: Répondre]        │
└──────────────────────────┘
```

### Notifications

- Un nouveau message fait clignoter l'icône ✉️ dans la barre d'état
- Le téléphone émet un bip sonore
- Le nombre de messages non lus est affiché

---

## Gérer les contacts

### Ajouter un contact

1. Appuyer sur `C` (Contacts)
2. Appuyer sur `D` (Ajouter)
3. Entrer le nom (mode texte)
4. Appuyer sur `*` pour passer au champ numéro
5. Entrer le numéro
6. Appuyer sur **VERT** pour enregistrer

### Modifier un contact

1. Sélectionner le contact dans la liste
2. Appuyer sur `C` pour voir les détails
3. Appuyer sur `C` à nouveau pour modifier
4. Modifier les champs
5. Appuyer sur **VERT** pour enregistrer

### Supprimer un contact

1. Sélectionner le contact
2. Appuyer sur `C` pour voir les détails
3. Appuyer sur `D` pour supprimer
4. Confirmer avec **VERT**

### Favoris

1. Dans les détails d'un contact
2. Appuyer sur `*` pour mettre en favori
3. Une étoile ★ apparaît à côté du nom

---

## Journal d'appels

### Consulter l'historique

1. Depuis l'accueil, appuyer sur `A` (Appels)
2. La liste des appels récents s'affiche

### Symboles du journal

| Symbole | Signification |
|---------|---------------|
| 🟢 → | Appel sortant |
| 🔵 ← | Appel entrant (répondu) |
| 🔴 ✗ | Appel manqué |
| 00:05:23 | Durée de l'appel |

### Actions depuis le journal

- Appuyer sur **VERT** pour rappeler le numéro
- Appuyer sur `C` pour ajouter aux contacts
- Appuyer sur `D` pour envoyer un SMS

---

## Paramètres

### Menu Réglages

```
┌──────────────────────────┐
│       RÉGLAGES           │
├──────────────────────────┤
│ > Son et vibrations     │
│ > Affichage             │
│ > Réseau LoRa           │
│ > Sécurité              │
│ > Énergie               │
│ > Système               │
└──────────────────────────┘
```

### Son et vibrations

| Option | Valeurs | Description |
|--------|---------|-------------|
| Volume sonnerie | 0-10 | Volume de la sonnerie |
| Sonnerie | 0-9 | Choix de la mélodie |
| Vibreur | ON/OFF | Activer le vibreur |
| Mode silencieux | ON/OFF | Couper tous les sons |

### Affichage

| Option | Valeurs | Description |
|--------|---------|-------------|
| Luminosité | 10-255 | Luminosité de l'écran |
| Extinction écran | 10-300s | Délai avant extinction |

### Réseau LoRa

| Option | Valeurs | Description |
|--------|---------|-------------|
| Fréquence | 868/915/433 MHz | Bande de fréquence |
| Puissance TX | 2-20 dBm | Puissance d'émission |

### Sécurité

| Option | Valeurs | Description |
|--------|---------|-------------|
| Code PIN | 4 chiffres | Verrouillage par code |
| Chiffrement | ON/OFF | Activer AES-128 |

---

## Saisie de texte (Multi-tap)

### Principe

Le téléphone utilise le système **multi-tap** pour écrire du texte avec les touches numériques, comme les anciens téléphones portables.

### Exemple : écrire "Salut"

| Appuis | Touche | Résultat |
|--------|--------|----------|
| 4 fois | 7 | S |
| 1 fois | 2 | a |
| 3 fois | 5 | l |
| 2 fois | 8 | u |
| 1 fois | 8 | t |

### Modes de saisie

| Mode | Touche J | Affichage | Comportement |
|------|----------|-----------|--------------|
| Minuscules | 1er appui | `abc` | lettres en minuscules |
| Majuscules | 2ème appui | `ABC` | LETTRES EN MAJUSCULES |
| Chiffres | 3ème appui | `123` | uniquement les chiffres |
| Symboles | 4ème appui | `!@#` | caractères spéciaux |

### Touches spéciales en mode texte

| Touche | Fonction |
|--------|----------|
| `I` | Caps Lock (verrouillage majuscule) |
| `J` | Changer de mode |
| `0` | Espace |
| `1` | Ponctuation (. , ! ?) |
| `C` | Effacer le dernier caractère |
| `*` | Insérer un symbole |

---

## Batterie et énergie

### Indicateurs de batterie

| Icône | Niveau | Signification |
|-------|--------|---------------|
| ████ | 75-100% | Batterie pleine |
| ███░ | 50-75% | Bon niveau |
| ██░░ | 25-50% | Niveau moyen |
| █░░░ | 10-25% | Batterie faible |
| ░░░░ | < 10% | Rechargez ! |
| ⚡ | - | En charge |

### Autonomie

| Usage | Autonomie estimée |
|-------|-------------------|
| Appel continu | ~16 heures |
| Normal (5 appels/jour) | ~3 jours |
| Veille légère | ~1 semaine |
| Veille profonde | ~1 mois |

### Économie d'énergie

Le téléphone gère automatiquement sa consommation :

1. **Après 30 secondes** : l'écran s'éteint
2. **Après 5 minutes** : mode veille légère
3. **Après 1 heure** : mode veille profonde

Pour maximiser l'autonomie :
- Réduire la luminosité de l'écran
- Désactiver le vibreur
- Réduire le volume
- Éteindre la torche après usage

---

## Sécurité

### Verrouillage par code PIN

1. Aller dans Réglages → Sécurité → Code PIN
2. Entrer le code PIN actuel (par défaut : `0000`)
3. Entrer le nouveau code PIN
4. Confirmer le nouveau code

### Changer le code PIN

1. Réglages → Sécurité → Code PIN
2. Entrer l'ancien code
3. Entrer le nouveau code (2 fois)

### En cas d'oubli du code PIN

⚠️ **Il n'y a pas de récupération possible !**
Si vous oubliez votre code PIN, vous devez faire un **reset usine** :
1. Éteindre le téléphone
2. Maintenir `*` + `#` + POWER
3. Le téléphone redémarre avec les paramètres d'usine
4. ⚠️ Toutes les données sont effacées !

### Chiffrement

Le chiffrement AES-128 peut être activé dans Réglages → Sécurité → Chiffrement.
Tous les téléphones du groupe doivent avoir le chiffrement activé avec la même clé.

---

## Dépannage

### Problèmes courants

| Problème | Solution |
|----------|----------|
| **Le téléphone ne s'allume pas** | Vérifier la batterie, charger 30 minutes |
| **Pas de signal** | Vérifier l'antenne, se rapprocher du correspondant |
| **Appel ne passe pas** | Vérifier le numéro, vérifier que le correspondant est allumé |
| **Son grésillant** | Réduire la distance, éviter les obstacles |
| **Écran noir** | Appuyer sur une touche pour réactiver l'écran |
| **Clavier ne répond pas** | Redémarrer le téléphone |
| **Batterie se vide vite** | Réduire la luminosité, désactiver le vibreur |
| **Message "Occupé"** | Le correspondant est déjà en appel |
| **Message "Hors ligne"** | Le correspondant est éteint ou hors de portée |

### Reset du téléphone

Si le téléphone est bloqué :

1. Maintenir le bouton POWER enfoncé 10 secondes
2. Le téléphone redémarre
3. Les données ne sont pas effacées

### Reset usine

Pour effacer toutes les données :

1. Éteindre le téléphone
2. Maintenir `C` + `D` + POWER
3. Confirmer avec VERT
4. Le téléphone redémarre comme neuf

---

## Foire aux questions

### Q : Quelle est la portée maximale ?

**R :** 5-15 km en terrain dégagé. La portée dépend du relief, des bâtiments et de la végétation. En ville, la portée est réduite à 1-3 km.

### Q : Combien de téléphones peuvent communiquer ensemble ?

**R :** Il n'y a pas de limite théorique. Tous les téléphones sur la même fréquence peuvent communiquer. En pratique, évitez de parler en même temps (half-duplex).

### Q : Les communications sont-elles privées ?

**R :** Sans chiffrement, n'importe qui avec un module LoRa peut écouter. Activez le chiffrement AES-128 pour sécuriser vos communications.

### Q : Puis-je utiliser le téléphone sous la pluie ?

**R :** Le téléphone n'est pas étanche. Protégez-le de l'humidité.

### Q : Comment savoir si quelqu'un est à portée ?

**R :** La barre de signal en haut de l'écran indique la force du signal. Vous pouvez aussi envoyer un "Ping" depuis le menu Réseau.

### Q : Puis-je recharger avec un panneau solaire ?

**R :** Oui, tout chargeur USB 5V fonctionne. Un petit panneau solaire USB est idéal pour une utilisation nomade.

### Q : Quelle est la durée de vie de la batterie ?

**R :** Environ 500 cycles de charge (2-3 ans d'utilisation normale). La batterie est remplaçable (format 18650 standard).

---

## 📚 Références

- [Architecture du projet](ARCHITECTURE.md)
- [Protocole de communication](PROTOCOL.md)
- [Schéma électronique](SCHEMATIC.md)
```

---

## ✅ FICHIER CRÉÉ

| Fichier | Lignes |
|---------|--------|
| `docs/USER_MANUAL.md` | ~700 |

---

## 🎉 DOCUMENTATION COMPLÈTE !

Le dossier `docs/` contient maintenant :

| Document | Lignes | Contenu |
|----------|--------|---------|
| `ARCHITECTURE.md` | ~600 | Architecture logicielle et matérielle |
| `PROTOCOL.md` | ~650 | Protocole de communication détaillé |
| `SCHEMATIC.md` | ~750 | Schéma électronique et câblage |
| `USER_MANUAL.md` | ~700 | Manuel utilisateur complet |
| **TOTAL** | **~2700** | **Documentation complète** |

---

## 📊 PROJET 100% TERMINÉ

| Catégorie | Fichiers | Lignes |
|-----------|----------|--------|
| Configuration | 2 | 300 |
| Drivers | 10 | 3 400 |
| Protocoles | 8 | 2 900 |
| Écrans | 12 | 3 100 |
| Widgets | 10 | 2 300 |
| Utilitaires | 10 | 2 280 |
| Principal | 1 | 600 |
| Tests | 4 | 1 780 |
| Documentation | 4 | 2 700 |
| **TOTAL** | **61** | **~19 360** |

Le Téléphone LoRa est maintenant entièrement documenté et prêt à être construit ! 🚀