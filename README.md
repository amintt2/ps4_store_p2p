# PS4 Store P2P 🎮

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-PS4-blue.svg)](https://www.playstation.com/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B17-red.svg)](https://isocpp.org/)
[![Build](https://img.shields.io/badge/Build-OpenOrbis-green.svg)](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain)

Un store décentralisé P2P pour PS4 jailbreakée, permettant de parcourir, télécharger et partager des homebrews et backups PS4 via un réseau peer-to-peer, similaire à Stremio ou Popcorn Time mais pour l'écosystème PS4.

## 🚀 Fonctionnalités

### ✅ Fonctionnalités Principales
- **Interface utilisateur moderne** avec SDL2 et ImGui
- **Téléchargement P2P** via libtorrent (BitTorrent)
- **Installation automatique** des packages PKG
- **Partage de contenu** entre utilisateurs
- **Vérification d'intégrité** des fichiers téléchargés
- **Gestion des téléchargements** avec pause/reprise
- **Recherche et filtrage** des contenus
- **Statistiques détaillées** de téléchargement/upload

### 🔧 Fonctionnalités Techniques
- **Architecture modulaire** (UI, P2P, PKG, Utils)
- **Optimisé pour PS4** uniquement
- **Configuration flexible** via fichiers INI
- **Logging avancé** pour le débogage
- **Documentation technique** détaillée

## 📋 Prérequis

### Pour la PS4
- **PS4 jailbreakée** (firmware ≤ 9.00 recommandé)
- **GoldHEN** ou équivalent installé
- **Debug Settings** activés
- **Connexion réseau** stable

### Pour le développement
- **OpenOrbis PS4 Toolchain**
- **SDL2** (≥ 2.0.20)
- **libtorrent** (≥ 2.0)
- **Dear ImGui** (≥ 1.89)
- **nlohmann/json** (≥ 3.11)
- **CMake** (≥ 3.16) ou **Make**

## 🛠️ Installation

### Installation Rapide (Linux/Codespace)

```bash
# Cloner le projet
git clone https://github.com/amintt2/ps4_store_p2p.git
cd ps4_store_p2p

# Extraire le toolchain OpenOrbis
chmod +x extract_openorbis.sh
./extract_openorbis.sh

# Compiler le projet
mkdir build && cd build
cmake ..
make
```

### Installation Manuelle

1. **Installer OpenOrbis Toolchain**
   ```bash
   git clone https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain.git
   cd OpenOrbis-PS4-Toolchain
   make install
   ```

2. **Installer les dépendances**
   ```bash
   make deps
   ```

3. **Compiler le projet**
   ```bash
   make ps4
   ```

4. **Créer le package PKG**
   ```bash
   make package
   ```

## 🎯 Utilisation

### Compilation

```bash
# Compilation pour PS4
make ps4

# Compilation native (pour tests)
make native

# Tests unitaires
make test

# Package PKG
make package

# Installation sur PS4 (via FTP)
make install PS4_IP=192.168.1.100
```

### Configuration

Éditez le fichier `config.ini` pour personnaliser :

```ini
[Network]
listen_port = 6881
max_download_speed = 0  # 0 = illimité
max_upload_speed = 0

[Paths]
downloads_path = /user/app/downloads
install_path = /user/app

[UI]
resolution_width = 1920
resolution_height = 1080
language = fr
```

### Installation sur PS4

1. **Via USB :**
   - Copiez `ps4_store_p2p.pkg` sur une clé USB
   - Insérez la clé dans la PS4
   - Allez dans **Debug Settings > Package Installer**
   - Sélectionnez le fichier PKG

2. **Via FTP :**
   ```bash
   make install PS4_IP=192.168.1.100
   ```

3. **Via navigateur PS4 :**
   - Hébergez le PKG sur un serveur web
   - Ouvrez le navigateur PS4
   - Naviguez vers l'URL du PKG

## 📁 Structure du Projet

```
ps4_store_p2p/
├── src/                    # Code source
│   ├── main.cpp           # Point d'entrée principal
│   ├── ui/                # Interface utilisateur
│   ├── p2p/               # Gestionnaire P2P/BitTorrent
│   ├── pkg/               # Gestionnaire de packages PKG
│   └── utils/             # Utilitaires généraux
├── include/               # Headers
│   ├── ui/
│   ├── p2p/
│   ├── pkg/
│   └── utils/
├── tests/                 # Tests unitaires
├── assets/                # Ressources (icônes, fonts)
├── docs/                  # Documentation
├── tools/                 # Scripts utilitaires
├── libs/                  # Bibliothèques externes
├── build/                 # Fichiers de build
├── CMakeLists.txt         # Configuration CMake
├── Makefile              # Configuration Make
├── config.ini            # Configuration par défaut
└── README.md             # Ce fichier
```

## 🔧 Développement

### Tests

```bash
# Exécuter tous les tests
make test

# Tests spécifiques
./build/test/test_main
```

### Débogage

```bash
# Build de debug
make debug

# Avec logs détaillés
echo "[Logging]\nlevel = debug" >> config.ini
```

### Documentation

```bash
# Générer la documentation Doxygen
make docs

# Ouvrir la documentation
open docs/html/index.html
```

## 📚 Documentation

- **[Guide Utilisateur](docs/USER_GUIDE.md)** - Guide complet d'utilisation
- **[Documentation Technique](docs/TECHNICAL.md)** - Architecture et détails techniques
- **[API Reference](docs/html/index.html)** - Documentation Doxygen (après `make docs`)

## 🤝 Contribution

1. **Fork** le projet
2. **Créez** une branche pour votre fonctionnalité (`git checkout -b feature/AmazingFeature`)
3. **Committez** vos changements (`git commit -m 'Add some AmazingFeature'`)
4. **Push** vers la branche (`git push origin feature/AmazingFeature`)
5. **Ouvrez** une Pull Request

### Guidelines de Contribution

- Suivez le style de code existant
- Ajoutez des tests pour les nouvelles fonctionnalités
- Mettez à jour la documentation si nécessaire
- Vérifiez que tous les tests passent

## 🐛 Problèmes Connus

### Compilation
- **Erreur libtorrent** : Vérifiez que boost est installé
- **Erreur SDL2** : Assurez-vous que les headers sont dans le bon répertoire

### Runtime
- **Crash au démarrage** : Vérifiez que GoldHEN est actif
- **Pas de connexion P2P** : Vérifiez les paramètres réseau et firewall

### Solutions

Consultez le [Guide de Dépannage](docs/USER_GUIDE.md#dépannage) pour plus de détails.

## ⚖️ Légalité et Responsabilité

> **⚠️ AVERTISSEMENT IMPORTANT**
>
> Ce logiciel est fourni à des fins **éducatives et de recherche uniquement**.
> 
> - La distribution de contenu protégé par le droit d'auteur est **illégale**
> - Chaque utilisateur est **responsable** du contenu qu'il partage
> - Respectez les **lois locales** et les **droits d'auteur**
> - Utilisez uniquement avec du **contenu légal** (homebrews, demos, contenu libre)

## 📄 Licence

Ce projet est sous licence MIT. Voir le fichier [LICENSE](LICENSE) pour plus de détails.

### Licences des Dépendances

- **OpenOrbis PS4 Toolchain** : GPL-3.0
- **SDL2** : Zlib License
- **Dear ImGui** : MIT License
- **libtorrent** : BSD License
- **nlohmann/json** : MIT License

## 🙏 Remerciements

- **[OpenOrbis Team](https://github.com/OpenOrbis)** pour le toolchain PS4
- **[Scene PS4](https://www.psxhax.com/)** pour la communauté et les ressources
- **[libtorrent](https://libtorrent.org/)** pour l'implémentation BitTorrent
- **[SDL2](https://www.libsdl.org/)** pour la couche multimédia
- **[Dear ImGui](https://github.com/ocornut/imgui)** pour l'interface utilisateur

## 📞 Support

- **Issues GitHub** : [Signaler un bug](https://github.com/amintt2/ps4_store_p2p/issues)

## 🗺️ Roadmap

### Version 1.1
- [ ] Support des mises à jour automatiques
- [ ] Interface web pour gestion à distance
- [ ] Support des plugins

### Version 1.2
- [ ] Chiffrement des communications
- [ ] Système de réputation des utilisateurs
- [ ] Support des torrents privés

### Version 2.0
- [ ] Support PS5 (si jailbreak disponible)
- [ ] Interface mobile (Android/iOS)
- [ ] Streaming direct (sans téléchargement)

---

**Développé avec ❤️ pour la communauté PS4 Homebrew**

*Dernière mise à jour : Décembre 2024*