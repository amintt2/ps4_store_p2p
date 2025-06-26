# Guide d'Utilisation - PS4 Store P2P

## 🎮 Introduction

Bienvenue dans le guide d'utilisation du **PS4 Store P2P**, une application homebrew qui vous permet de télécharger et installer des packages PS4 via un réseau peer-to-peer décentralisé.

⚠️ **AVERTISSEMENT LÉGAL**

Cette application est destinée uniquement à des fins éducatives et de recherche. Vous êtes entièrement responsable du contenu que vous téléchargez et partagez. Respectez les lois sur le droit d'auteur de votre pays.

## 📋 Prérequis

### Matériel Requis
- **PS4** avec firmware jailbreakable (≤ 9.00 recommandé)
- **Espace disque** : Au moins 10 GB libres
- **Connexion Internet** : Haut débit recommandé
- **Clé USB** (pour l'installation initiale)

### Logiciels Requis
- **GoldHEN** installé et actif
- **Debug Settings** activé
- **FTP Server** (optionnel, pour l'installation via réseau)

## 🔧 Installation

### Méthode 1 : Installation via Clé USB

1. **Télécharger le Package**
   - Récupérez le fichier `ps4_store_p2p.pkg`
   - Copiez-le sur une clé USB formatée en FAT32

2. **Préparer la PS4**
   ```
   1. Assurez-vous que GoldHEN est chargé
   2. Allez dans Paramètres > Système > Debug Settings
   3. Activez "Enable Debug Menu"
   4. Redémarrez la PS4 si nécessaire
   ```

3. **Installer le Package**
   ```
   1. Insérez la clé USB dans la PS4
   2. Allez dans Debug Settings > Game > Package Installer
   3. Sélectionnez le fichier ps4_store_p2p.pkg
   4. Confirmez l'installation
   5. Attendez la fin de l'installation
   ```

### Méthode 2 : Installation via FTP

1. **Activer le FTP Server**
   - Lancez votre homebrew FTP favori sur la PS4
   - Notez l'adresse IP affichée

2. **Transférer le Package**
   ```bash
   # Depuis votre PC, utilisez un client FTP
   ftp 192.168.1.XXX  # IP de votre PS4
   # Uploadez ps4_store_p2p.pkg vers /data/
   ```

3. **Installer via Debug Settings**
   - Suivez les étapes 3 de la méthode USB
   - Le fichier sera dans `/data/ps4_store_p2p.pkg`

## 🚀 Premier Lancement

### Configuration Initiale

1. **Lancer l'Application**
   - Allez dans le menu principal de la PS4
   - Trouvez "PS4 Store P2P" dans vos applications
   - Lancez l'application

2. **Configuration Réseau**
   ```
   Au premier lancement, vous verrez l'écran de configuration :
   
   ┌─────────────────────────────────────┐
   │        Configuration Réseau         │
   ├─────────────────────────────────────┤
   │ Port d'écoute    : [6881]           │
   │ Limite Download  : [0] KB/s         │
   │ Limite Upload    : [512] KB/s       │
   │ Activer DHT      : [✓]              │
   │ Activer UPnP     : [✓]              │
   ├─────────────────────────────────────┤
   │ [Sauvegarder]  [Par Défaut]         │
   └─────────────────────────────────────┘
   ```

3. **Configuration des Chemins**
   - Les chemins par défaut sont optimaux
   - Modifiez uniquement si nécessaire
   - Assurez-vous d'avoir les permissions d'écriture

## 🎯 Utilisation de l'Interface

### Menu Principal

```
┌─────────────────────────────────────────────────────────────┐
│                    PS4 Store P2P v1.0                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  🎮 [Parcourir les Jeux]     📥 [Mes Téléchargements]      │
│                                                             │
│  📦 [Mes Packages]           ⚙️  [Paramètres]              │
│                                                             │
│  📊 [Statistiques]           ❓ [Aide]                     │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│ Statut : Connecté | Peers : 42 | Vitesse : 2.5 MB/s         │
└─────────────────────────────────────────────────────────────┘
```

### Navigation

- **Stick Gauche** : Navigation dans les menus
- **Bouton X** : Sélectionner/Confirmer
- **Bouton O** : Retour/Annuler
- **Bouton Triangle** : Menu contextuel
- **Bouton Carré** : Actions rapides
- **L1/R1** : Changer d'onglet
- **L2/R2** : Défiler rapidement

## 📚 Fonctionnalités Principales

### 1. Parcourir les Jeux

```
┌─────────────────────────────────────────────────────────────┐
│                     Liste des Jeux                          │
├─────────────────────────────────────────────────────────────┤
│ 🔍 [Rechercher...]                    📊 Trier par: [Nom]  │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ 🎮 The Last of Us Part II              📦 45.2 GB          │
│    Action/Aventure | ⭐⭐⭐⭐⭐ | 👥 1,234 seeders        │
│    [Télécharger] [Détails] [Partager]                       │
│                                                             │
│ 🏎️ Gran Turismo 7                      📦 89.1 GB          │
│    Course/Sport | ⭐⭐⭐⭐ | 👥 856 seeders               │
│    [Télécharger] [Détails] [Partager]                       │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

#### Actions Disponibles
- **Télécharger** : Démarre le téléchargement P2P
- **Détails** : Affiche les informations complètes
- **Partager** : Partage votre copie si vous l'avez
- **Rechercher** : Filtre par nom, genre, etc.

### 2. Gestionnaire de Téléchargements

```
┌─────────────────────────────────────────────────────────────┐
│                   Téléchargements Actifs                    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ 📥 The Last of Us Part II                                   │
│    ████████████████████████████████████░░░░░ 85%            │
│    38.4 GB / 45.2 GB | ⬇️ 2.1 MB/s | ⏱️ 5m restantes        │
│    👥 12 peers | 🌱 8 seeders                               │
│    [Pause] [Arrêter] [Priorité +]                           │
│                                                             │
│ 📥 Gran Turismo 7                                           │
│    ████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 12%           │
│    10.7 GB / 89.1 GB | ⬇️ 850 KB/s | ⏱️ 2h 15m restantes   │
│    👥 5 peers | 🌱 3 seeders                               │
│    [Pause] [Arrêter] [Priorité -]                           │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

#### Contrôles de Téléchargement
- **Pause/Reprendre** : Contrôle individuel
- **Arrêter** : Annule le téléchargement
- **Priorité** : Ajuste l'ordre de téléchargement
- **Détails** : Informations techniques détaillées

### 3. Mes Packages

```
┌─────────────────────────────────────────────────────────────┐
│                    Packages Installés                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ ✅ Spider-Man: Miles Morales       📦 50.1 GB | 🔄 Partagé │
│    Installé le 15/03/2024                                   │
│    [Lancer] [Désinstaller] [Arrêter Partage]                │
│                                                             │
│ ✅ Horizon Zero Dawn               📦 67.3 GB | ❌ Local   │
│    Installé le 10/03/2024                                   │
│    [Lancer] [Désinstaller] [Commencer Partage]              │
│                                                             │
│ ⏳ The Last of Us Part II          📦 45.2 GB | 🔄 Install │
│    Installation en cours... 78%                             │
│    [Voir Progression] [Annuler]                             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

#### Gestion des Packages
- **Lancer** : Démarre le jeu
- **Désinstaller** : Supprime le package
- **Partager** : Met le package en partage P2P
- **Arrêter Partage** : Arrête le partage

### 4. Paramètres

#### Onglet Réseau
```
┌─────────────────────────────────────────────────────────────┐
│                    Paramètres Réseau                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ Port d'écoute P2P        : [6881]                           │
│ Limite téléchargement    : [0] KB/s (illimité)              │
│ Limite upload            : [512] KB/s                       │
│ Connexions simultanées   : [50]                             │
│                                                             │
│ ☑️ Activer DHT                                              │
│ ☑️ Activer découverte locale (LSD)                          │
│ ☑️ Activer UPnP                                             │
│ ☑️ Chiffrement forcé                                        │
│                                                             │
│ Trackers personnalisés   : [Modifier...]                    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

#### Onglet Stockage
```
┌─────────────────────────────────────────────────────────────┐
│                   Paramètres Stockage                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ Dossier téléchargements  : /data/ps4_store/downloads        │
│ Dossier temporaire       : /data/ps4_store/temp             │
│ Dossier cache            : /data/ps4_store/cache            │
│                                                             │
│ Espace disque utilisé    : 156.7 GB / 500 GB (31%)          │
│ Cache métadonnées        : 2.3 GB                           │
│ Fichiers temporaires     : 847 MB                           │
│                                                             │
│ ☑️ Nettoyage automatique des fichiers temporaires           │
│ ☑️ Vérification d'intégrité automatique                     │
│                                                             │
│ [Nettoyer Cache] [Nettoyer Temp] [Analyser Disque]          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## 🔍 Fonctionnalités Avancées

### Recherche Avancée

```
┌─────────────────────────────────────────────────────────────┐
│                    Recherche Avancée                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ Nom du jeu     : [___________________________]              │
│ Genre          : [Tous ▼]                                   │
│ Taille min     : [____] GB                                  │
│ Taille max     : [____] GB                                  │
│ Note minimum   : [⭐⭐⭐☆☆]                                │
│ Seeders min    : [____]                                     │
│                                                             │
│ ☐ Jeux avec DLC inclus                                      │
│ ☐ Versions mises à jour uniquement                          │
│ ☐ Contenu en français disponible                            │
│                                                             │
│ [Rechercher] [Réinitialiser] [Sauver Filtre]                │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Statistiques Détaillées

```
┌─────────────────────────────────────────────────────────────┐
│                      Statistiques                           │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ 📊 Session Actuelle                                         │
│    Téléchargé    : 12.7 GB                                  │
│    Uploadé       : 3.2 GB                                   │
│    Ratio         : 0.25                                     │
│    Durée         : 2h 34m                                   │
│                                                             │
│ 📈 Statistiques Globales                                    │
│    Total téléchargé : 847.3 GB                              │
│    Total uploadé    : 234.1 GB                              │
│    Ratio global     : 0.28                                  │
│    Jeux installés   : 23                                    │
│    Jeux partagés    : 15                                    │
│                                                             │
│ 🌐 Réseau                                                   │
│    Peers connectés  : 42                                    │
│    Torrents actifs  : 8                                     │
│    DHT nodes        : 1,247                                 │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## 🛠️ Dépannage

### Problèmes Courants

#### 1. L'application ne se lance pas

**Symptômes :**
- Écran noir au lancement
- Retour au menu PS4
- Message d'erreur

**Solutions :**
```
1. Vérifiez que GoldHEN est actif
   - Relancez GoldHEN si nécessaire
   - Vérifiez la version (2.2.5+ recommandée)

2. Contrôlez l'installation
   - Réinstallez le package PKG
   - Vérifiez l'intégrité du fichier

3. Libérez de l'espace disque
   - Au moins 1 GB libre requis
   - Supprimez les fichiers temporaires
```

#### 2. Pas de connexion P2P

**Symptômes :**
- Aucun peer trouvé
- Téléchargements qui ne démarrent pas
- Vitesse de téléchargement nulle

**Solutions :**
```
1. Vérifiez la connexion Internet
   - Testez avec le navigateur PS4
   - Vérifiez les paramètres réseau

2. Configurez le routeur
   - Activez UPnP si possible
   - Ouvrez le port 6881 (ou celui configuré)

3. Changez les paramètres P2P
   - Essayez un autre port
   - Désactivez temporairement le chiffrement
   - Ajoutez des trackers manuellement
```

#### 3. Échec d'installation des packages

**Symptômes :**
- Erreur lors de l'installation
- Package corrompu
- Installation qui s'arrête

**Solutions :**
```
1. Vérifiez l'intégrité
   - Relancez la vérification SHA256
   - Retéléchargez si nécessaire

2. Libérez de l'espace
   - Vérifiez l'espace disque disponible
   - Supprimez des jeux non utilisés

3. Réinitialisez Debug Settings
   - Désactivez puis réactivez
   - Redémarrez la PS4
```

### Codes d'Erreur

| Code | Description | Solution |
|------|-------------|----------|
| E001 | Erreur d'initialisation SDL | Redémarrer l'application |
| E002 | Échec de connexion réseau | Vérifier la connexion Internet |
| E003 | Erreur libtorrent | Réinstaller l'application |
| E004 | Espace disque insuffisant | Libérer de l'espace |
| E005 | Package corrompu | Retélécharger le fichier |
| E006 | Permissions insuffisantes | Vérifier GoldHEN |
| E007 | Erreur d'installation | Réessayer via Debug Settings |

## 📞 Support et Communauté

### Obtenir de l'Aide

1. **Documentation**
   - Consultez ce guide en premier
   - Lisez la documentation technique
   - Vérifiez les FAQ

2. **Logs de Débogage**
   ```
   Les logs sont sauvegardés dans :
   /data/ps4_store/ps4_store.log
   
   Pour activer le mode debug :
   Paramètres > Avancé > Mode Debug
   ```

3. **Communauté**
   - Forums de homebrew PS4
   - Discord communautaire
   - GitHub Issues (pour les bugs)

### Contribuer au Projet

- **Rapporter des bugs** : Utilisez GitHub Issues
- **Proposer des améliorations** : Pull Requests bienvenues
- **Traductions** : Aidez à traduire l'interface
- **Documentation** : Améliorez ce guide

## 🔒 Sécurité et Bonnes Pratiques

### Sécurité

1. **Ne téléchargez que du contenu légal**
2. **Vérifiez toujours l'intégrité des fichiers**
3. **Utilisez des sources fiables**
4. **Maintenez votre système à jour**

### Bonnes Pratiques P2P

1. **Partagez ce que vous téléchargez**
   - Maintenez un bon ratio upload/download
   - Laissez les torrents en seed après téléchargement

2. **Respectez les limites de bande passante**
   - Configurez des limites raisonnables
   - Ne saturez pas votre connexion

3. **Soyez patient**
   - Les téléchargements P2P peuvent prendre du temps
   - Plus il y a de seeders, plus c'est rapide

## 📋 FAQ

### Questions Générales

**Q : L'application fonctionne-t-elle sur toutes les PS4 ?**
R : Non, uniquement sur les PS4 jailbreakées avec firmware ≤ 9.00 et GoldHEN installé.

**Q : Puis-je utiliser l'application en ligne avec Sony ?**
R : Non, l'utilisation d'homebrews peut entraîner un bannissement. Utilisez un compte secondaire.

**Q : L'application ralentit-elle ma PS4 ?**
R : L'impact est minimal. Vous pouvez limiter l'utilisation CPU/réseau dans les paramètres.

**Q : Puis-je télécharger plusieurs jeux simultanément ?**
R : Oui, jusqu'à 3 téléchargements simultanés par défaut (configurable).

### Questions Techniques

**Q : Quels formats de fichiers sont supportés ?**
R : Uniquement les fichiers .pkg PS4 valides.

**Q : L'application supporte-t-elle les DLC ?**
R : Oui, si le package inclut les DLC ou s'ils sont distribués séparément.

**Q : Puis-je changer l'emplacement d'installation ?**
R : L'installation se fait toujours dans /user/app (limitation PS4).

---

## 📄 Licence et Crédits

### Licence
Ce projet est distribué sous licence MIT. Voir le fichier LICENSE pour plus de détails.

### Crédits
- **OpenOrbis Team** : SDK PS4 homebrew
- **SDL Team** : Bibliothèque graphique
- **ImGui Team** : Interface utilisateur
- **libtorrent Team** : Moteur P2P
- **Communauté PS4 Homebrew** : Support et tests

### Avertissement Final

⚠️ **UTILISEZ À VOS PROPRES RISQUES**

Les développeurs ne sont pas responsables de :
- Dommages à votre console
- Bannissements de comptes
- Problèmes légaux liés au contenu
- Perte de données

Cette application est fournie "en l'état" sans garantie d'aucune sorte.

---

*Guide d'utilisation PS4 Store P2P v1.0 - Dernière mise à jour : Mars 2024*