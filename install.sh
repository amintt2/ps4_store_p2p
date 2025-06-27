#!/bin/bash
# Script d'installation pour PS4 Store P2P
# Ce script configure l'environnement de développement et compile le projet

set -e  # Arrêter en cas d'erreur

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Variables de configuration
PROJECT_NAME="PS4 Store P2P"
OPENORBIS_URL="https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain.git"
SDL2_VERSION="2.28.5"
IMGUI_VERSION="1.90.1"
LIBTORRENT_VERSION="2.0.9"

# Fonction d'affichage avec couleurs
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Vérification des prérequis
check_prerequisites() {
    print_status "Vérification des prérequis..."
    
    # Vérifier Git
    if ! command -v git &> /dev/null; then
        print_error "Git n'est pas installé. Veuillez l'installer d'abord."
        exit 1
    fi
    
    # Vérifier CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake n'est pas installé. Veuillez l'installer d'abord."
        exit 1
    fi
    
    # Vérifier Make
    if ! command -v make &> /dev/null; then
        print_error "Make n'est pas installé. Veuillez l'installer d'abord."
        exit 1
    fi
    
    print_success "Tous les prérequis sont installés."
}

# Installation d'OpenOrbis Toolchain
install_openorbis() {
    print_status "Installation d'OpenOrbis Toolchain..."
    
    if [ ! -d "OpenOrbis-PS4-Toolchain" ]; then
        git clone $OPENORBIS_URL
        cd OpenOrbis-PS4-Toolchain
        
        # Configuration et compilation
        mkdir -p build
        cd build
        cmake ..
        make -j$(nproc)
        
        # Installation
        sudo make install
        cd ../..
        
        print_success "OpenOrbis Toolchain installé avec succès."
    else
        print_warning "OpenOrbis Toolchain déjà présent."
    fi
}

# Téléchargement et compilation de SDL2
install_sdl2() {
    print_status "Installation de SDL2 v$SDL2_VERSION..."
    
    if [ ! -d "libs/SDL2" ]; then
        mkdir -p libs
        cd libs
        
        # Télécharger SDL2
        wget "https://github.com/libsdl-org/SDL/releases/download/release-$SDL2_VERSION/SDL2-$SDL2_VERSION.tar.gz"
        tar -xzf "SDL2-$SDL2_VERSION.tar.gz"
        mv "SDL2-$SDL2_VERSION" SDL2
        rm "SDL2-$SDL2_VERSION.tar.gz"
        
        cd SDL2
        
        # Configuration pour PS4
        mkdir -p build
        cd build
        
        # Cross-compilation pour PS4
        cmake .. \
            -DCMAKE_TOOLCHAIN_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/PS4Toolchain.cmake" \
            -DCMAKE_BUILD_TYPE=Release \
            -DSDL_SHARED=OFF \
            -DSDL_STATIC=ON \
            -DSDL_TEST=OFF \
            -DSDL_TESTS=OFF
        
        make -j$(nproc)
        cd ../../..
        
        print_success "SDL2 compilé avec succès."
    else
        print_warning "SDL2 déjà présent."
    fi
}

# Téléchargement et compilation d'ImGui
install_imgui() {
    print_status "Installation d'ImGui v$IMGUI_VERSION..."
    
    if [ ! -d "libs/imgui" ]; then
        mkdir -p libs
        cd libs
        
        # Télécharger ImGui
        wget "https://github.com/ocornut/imgui/archive/v$IMGUI_VERSION.tar.gz"
        tar -xzf "v$IMGUI_VERSION.tar.gz"
        mv "imgui-$IMGUI_VERSION" imgui
        rm "v$IMGUI_VERSION.tar.gz"
        
        cd ..
        print_success "ImGui téléchargé avec succès."
    else
        print_warning "ImGui déjà présent."
    fi
}

# Téléchargement et compilation de libtorrent
install_libtorrent() {
    print_status "Installation de libtorrent v$LIBTORRENT_VERSION..."
    
    if [ ! -d "libs/libtorrent" ]; then
        mkdir -p libs
        cd libs
        
        # Télécharger libtorrent
        wget "https://github.com/arvidn/libtorrent/releases/download/v$LIBTORRENT_VERSION/libtorrent-rasterbar-$LIBTORRENT_VERSION.tar.gz"
        tar -xzf "libtorrent-rasterbar-$LIBTORRENT_VERSION.tar.gz"
        mv "libtorrent-rasterbar-$LIBTORRENT_VERSION" libtorrent
        rm "libtorrent-rasterbar-$LIBTORRENT_VERSION.tar.gz"
        
        cd libtorrent
        
        # Configuration pour PS4
        mkdir -p build
        cd build
        
        cmake .. \
            -DCMAKE_TOOLCHAIN_FILE=../../cmake/PS4Toolchain.cmake \
            -DCMAKE_BUILD_TYPE=Release \
            -Dstatic_runtime=ON \
            -Dbuild_shared_libs=OFF
        
        make -j$(nproc)
        cd ../../..
        
        print_success "libtorrent compilé avec succès."
    else
        print_warning "libtorrent déjà présent."
    fi
}

# Création du fichier de toolchain CMake pour PS4
create_cmake_toolchain() {
    print_status "Création du fichier de toolchain CMake..."
    
    mkdir -p cmake
    cat > cmake/PS4Toolchain.cmake << 'EOF'
# Toolchain file for PS4 cross-compilation
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Spécifier le compilateur
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Flags pour PS4
set(PS4_FLAGS "-target x86_64-pc-freebsd12-elf -fPIC -funwind-tables")
set(CMAKE_C_FLAGS "${PS4_FLAGS}")
set(CMAKE_CXX_FLAGS "${PS4_FLAGS} -std=c++17")

# Désactiver les tests pour les bibliothèques
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR "")
EOF
    
    print_success "Fichier de toolchain CMake créé."
}

# Compilation du projet principal
build_project() {
    print_status "Compilation du projet $PROJECT_NAME..."
    
    # Nettoyer le build précédent
    rm -rf build
    mkdir -p build
    
    # Compiler avec Make
    make clean
    make -j$(nproc)
    
    if [ $? -eq 0 ]; then
        print_success "Projet compilé avec succès!"
        print_status "Fichier généré: build/ps4_store_p2p.elf"
    else
        print_error "Erreur lors de la compilation."
        exit 1
    fi
}

# Création du package PKG
create_package() {
    print_status "Création du package PKG..."
    
    if [ -f "build/ps4_store_p2p.elf" ]; then
        # Utiliser les outils OpenOrbis pour créer le PKG
        orbis-pub-gen project.gp4 build/
        
        if [ $? -eq 0 ]; then
            print_success "Package PKG créé avec succès!"
            print_status "Fichier généré: build/ps4_store_p2p.pkg"
        else
            print_error "Erreur lors de la création du package."
        fi
    else
        print_error "Fichier ELF non trouvé. Compilez d'abord le projet."
    fi
}

# Fonction d'aide
show_help() {
    echo "Usage: $0 [OPTION]"
    echo "Script d'installation pour $PROJECT_NAME"
    echo ""
    echo "Options:"
    echo "  --deps      Installer uniquement les dépendances"
    echo "  --build     Compiler uniquement le projet"
    echo "  --package   Créer uniquement le package PKG"
    echo "  --clean     Nettoyer les fichiers de build"
    echo "  --help      Afficher cette aide"
    echo ""
    echo "Sans option, installe tout et compile le projet."
}

# Nettoyage
clean_build() {
    print_status "Nettoyage des fichiers de build..."
    rm -rf build
    make clean
    print_success "Nettoyage terminé."
}

# Fonction principale
main() {
    echo "=========================================="
    echo "    Installation de $PROJECT_NAME"
    echo "=========================================="
    echo ""
    
    case "${1:-}" in
        --deps)
            check_prerequisites
            create_cmake_toolchain
            install_openorbis
            install_sdl2
            install_imgui
            install_libtorrent
            ;;
        --build)
            build_project
            ;;
        --package)
            create_package
            ;;
        --clean)
            clean_build
            ;;
        --help)
            show_help
            ;;
        "")
            check_prerequisites
            create_cmake_toolchain
            install_openorbis
            install_sdl2
            install_imgui
            install_libtorrent
            build_project
            create_package
            ;;
        *)
            print_error "Option inconnue: $1"
            show_help
            exit 1
            ;;
    esac
    
    echo ""
    print_success "Installation terminée!"
    echo "=========================================="
}

# Exécuter la fonction principale avec tous les arguments
main "$@"