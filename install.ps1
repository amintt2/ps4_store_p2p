# Script d'installation PowerShell pour PS4 Store P2P
# Compatible Windows avec WSL ou environnement de développement natif

param(
    [string]$Action = "all",
    [switch]$Help,
    [switch]$Verbose
)

# Configuration
$ProjectName = "PS4 Store P2P"
$OpenOrbisUrl = "https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain.git"
$SDL2Version = "2.28.5"
$ImGuiVersion = "1.90.1"
$LibTorrentVersion = "2.0.9"
$JsonVersion = "3.11.3"

# Couleurs pour l'affichage
function Write-ColorOutput {
    param(
        [string]$Message,
        [string]$Color = "White"
    )
    Write-Host $Message -ForegroundColor $Color
}

function Write-Status {
    param([string]$Message)
    Write-ColorOutput "[INFO] $Message" "Cyan"
}

function Write-Success {
    param([string]$Message)
    Write-ColorOutput "[SUCCESS] $Message" "Green"
}

function Write-Warning {
    param([string]$Message)
    Write-ColorOutput "[WARNING] $Message" "Yellow"
}

function Write-Error {
    param([string]$Message)
    Write-ColorOutput "[ERROR] $Message" "Red"
}

# Fonction d'aide
function Show-Help {
    Write-Host @"
$ProjectName - Script d'Installation

Usage: .\install.ps1 [ACTION] [OPTIONS]

Actions:
  all         Installation complète (par défaut)
  deps        Installer uniquement les dépendances
  build       Compiler uniquement le projet
  clean       Nettoyer les fichiers de build
  setup-wsl   Configurer WSL pour le développement
  help        Afficher cette aide

Options:
  -Verbose    Affichage détaillé
  -Help       Afficher cette aide

Exemples:
  .\install.ps1                    # Installation complète
  .\install.ps1 deps               # Installer les dépendances
  .\install.ps1 build -Verbose     # Compiler avec sortie détaillée
  .\install.ps1 clean              # Nettoyer le projet

"@
}

# Vérification des prérequis
function Test-Prerequisites {
    Write-Status "Vérification des prérequis..."
    
    $missing = @()
    
    # Vérifier Git
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        $missing += "Git"
    }
    
    # Vérifier WSL (pour OpenOrbis)
    try {
        $wslInfo = wsl --list --quiet 2>$null
        if (-not $wslInfo) {
            Write-Warning "WSL n'est pas configuré. OpenOrbis nécessite un environnement Linux."
        }
    } catch {
        Write-Warning "WSL n'est pas disponible. Considérez l'installation de WSL pour OpenOrbis."
    }
    
    # Vérifier Visual Studio Build Tools ou Visual Studio
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsInstalls = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
        if (-not $vsInstalls) {
            $missing += "Visual Studio Build Tools (C++)"
        }
    } else {
        $missing += "Visual Studio ou Build Tools"
    }
    
    if ($missing.Count -gt 0) {
        Write-Error "Prérequis manquants: $($missing -join ', ')"
        Write-Host @"

Pour installer les prérequis manquants:

1. Git: https://git-scm.com/download/win
2. Visual Studio Community: https://visualstudio.microsoft.com/fr/vs/community/
   - Sélectionnez "Développement Desktop en C++"
3. WSL (optionnel pour OpenOrbis): wsl --install

"@
        return $false
    }
    
    Write-Success "Tous les prérequis sont installés."
    return $true
}

# Configuration WSL pour OpenOrbis
function Setup-WSL {
    Write-Status "Configuration de WSL pour OpenOrbis..."
    
    try {
        # Vérifier si WSL est installé
        $wslDistros = wsl --list --quiet 2>$null
        if (-not $wslDistros) {
            Write-Status "Installation de WSL..."
            wsl --install -d Ubuntu
            Write-Warning "WSL a été installé. Redémarrez votre ordinateur et relancez ce script."
            return
        }
        
        Write-Status "Mise à jour des packages WSL..."
        wsl sudo apt update
        wsl sudo apt upgrade -y
        
        Write-Status "Installation des outils de développement..."
        wsl sudo apt install -y build-essential cmake git clang llvm
        
        Write-Success "WSL configuré avec succès."
    } catch {
        Write-Error "Erreur lors de la configuration de WSL: $($_.Exception.Message)"
    }
}

# Installation d'OpenOrbis Toolchain
function Install-OpenOrbis {
    Write-Status "Installation d'OpenOrbis Toolchain..."
    
    if (Test-Path "OpenOrbis-PS4-Toolchain") {
        Write-Warning "OpenOrbis Toolchain déjà présent."
        return
    }
    
    try {
        # Cloner le repository
        git clone $OpenOrbisUrl
        
        if (Get-Command wsl -ErrorAction SilentlyContinue) {
            Write-Status "Compilation d'OpenOrbis via WSL..."
            wsl bash -c "cd OpenOrbis-PS4-Toolchain && mkdir -p build && cd build && cmake .. && make -j`$(nproc)"
        } else {
            Write-Warning "WSL non disponible. Compilation manuelle requise."
            Write-Host @"
Pour compiler OpenOrbis manuellement:
1. Ouvrez WSL ou un environnement Linux
2. cd OpenOrbis-PS4-Toolchain
3. mkdir build && cd build
4. cmake ..
5. make -j`$(nproc)
"@
        }
        
        Write-Success "OpenOrbis Toolchain installé."
    } catch {
        Write-Error "Erreur lors de l'installation d'OpenOrbis: $($_.Exception.Message)"
    }
}

# Téléchargement et extraction de SDL2
function Install-SDL2 {
    Write-Status "Installation de SDL2 v$SDL2Version..."
    
    $sdlDir = "libs\SDL2"
    if (Test-Path $sdlDir) {
        Write-Warning "SDL2 déjà présent."
        return
    }
    
    try {
        New-Item -ItemType Directory -Force -Path "libs" | Out-Null
        
        $sdlUrl = "https://github.com/libsdl-org/SDL/releases/download/release-$SDL2Version/SDL2-devel-$SDL2Version-VC.zip"
        $sdlZip = "libs\SDL2-$SDL2Version.zip"
        
        Write-Status "Téléchargement de SDL2..."
        Invoke-WebRequest -Uri $sdlUrl -OutFile $sdlZip
        
        Write-Status "Extraction de SDL2..."
        Expand-Archive -Path $sdlZip -DestinationPath "libs"
        Rename-Item "libs\SDL2-$SDL2Version" "libs\SDL2"
        Remove-Item $sdlZip
        
        Write-Success "SDL2 installé avec succès."
    } catch {
        Write-Error "Erreur lors de l'installation de SDL2: $($_.Exception.Message)"
    }
}

# Téléchargement d'ImGui
function Install-ImGui {
    Write-Status "Installation d'ImGui v$ImGuiVersion..."
    
    $imguiDir = "libs\imgui"
    if (Test-Path $imguiDir) {
        Write-Warning "ImGui déjà présent."
        return
    }
    
    try {
        New-Item -ItemType Directory -Force -Path "libs" | Out-Null
        
        $imguiUrl = "https://github.com/ocornut/imgui/archive/v$ImGuiVersion.zip"
        $imguiZip = "libs\imgui-$ImGuiVersion.zip"
        
        Write-Status "Téléchargement d'ImGui..."
        Invoke-WebRequest -Uri $imguiUrl -OutFile $imguiZip
        
        Write-Status "Extraction d'ImGui..."
        Expand-Archive -Path $imguiZip -DestinationPath "libs"
        Rename-Item "libs\imgui-$ImGuiVersion" "libs\imgui"
        Remove-Item $imguiZip
        
        Write-Success "ImGui installé avec succès."
    } catch {
        Write-Error "Erreur lors de l'installation d'ImGui: $($_.Exception.Message)"
    }
}

# Téléchargement de JSON for Modern C++
function Install-Json {
    Write-Status "Installation de JSON for Modern C++ v$JsonVersion..."
    
    $jsonDir = "libs\json"
    if (Test-Path $jsonDir) {
        Write-Warning "JSON library déjà présente."
        return
    }
    
    try {
        New-Item -ItemType Directory -Force -Path "libs\json" | Out-Null
        
        $jsonUrl = "https://github.com/nlohmann/json/releases/download/v$JsonVersion/json.hpp"
        $jsonFile = "libs\json\json.hpp"
        
        Write-Status "Téléchargement de JSON library..."
        Invoke-WebRequest -Uri $jsonUrl -OutFile $jsonFile
        
        Write-Success "JSON library installée avec succès."
    } catch {
        Write-Error "Erreur lors de l'installation de JSON library: $($_.Exception.Message)"
    }
}

# Installation de libtorrent (version Windows pour développement)
function Install-LibTorrent {
    Write-Status "Installation de libtorrent v$LibTorrentVersion..."
    
    $torrentDir = "libs\libtorrent"
    if (Test-Path $torrentDir) {
        Write-Warning "libtorrent déjà présent."
        return
    }
    
    try {
        Write-Status "Clonage de libtorrent..."
        git clone --branch "v$LibTorrentVersion" --depth 1 "https://github.com/arvidn/libtorrent.git" $torrentDir
        
        Write-Warning "libtorrent téléchargé. Compilation manuelle requise."
        Write-Host @"
Pour compiler libtorrent:
1. Ouvrez Visual Studio Developer Command Prompt
2. cd $torrentDir
3. mkdir build && cd build
4. cmake .. -DCMAKE_BUILD_TYPE=Release
5. cmake --build . --config Release
"@
        
        Write-Success "libtorrent téléchargé."
    } catch {
        Write-Error "Erreur lors de l'installation de libtorrent: $($_.Exception.Message)"
    }
}

# Compilation du projet
function Build-Project {
    Write-Status "Compilation du projet $ProjectName..."
    
    try {
        # Nettoyer le build précédent
        if (Test-Path "build") {
            Remove-Item -Recurse -Force "build"
        }
        New-Item -ItemType Directory -Force -Path "build" | Out-Null
        
        # Utiliser CMake si disponible
        if (Get-Command cmake -ErrorAction SilentlyContinue) {
            Write-Status "Compilation avec CMake..."
            Set-Location "build"
            cmake ..
            cmake --build . --config Release
            Set-Location ".."
        } else {
            Write-Status "Compilation avec Make (via WSL)..."
            if (Get-Command wsl -ErrorAction SilentlyContinue) {
                wsl make clean
                wsl make -j
            } else {
                Write-Error "Ni CMake ni WSL disponible pour la compilation."
                return
            }
        }
        
        Write-Success "Projet compilé avec succès!"
        
        if (Test-Path "build\ps4_store_p2p.elf") {
            Write-Status "Fichier généré: build\ps4_store_p2p.elf"
        }
    } catch {
        Write-Error "Erreur lors de la compilation: $($_.Exception.Message)"
    }
}

# Nettoyage
function Clean-Build {
    Write-Status "Nettoyage des fichiers de build..."
    
    $itemsToClean = @("build", "bin", "obj", "*.elf", "*.pkg", "*.o", "*.obj")
    
    foreach ($item in $itemsToClean) {
        if (Test-Path $item) {
            Remove-Item -Recurse -Force $item
            Write-Status "Supprimé: $item"
        }
    }
    
    # Nettoyer avec Make si disponible
    if ((Get-Command wsl -ErrorAction SilentlyContinue) -and (Test-Path "Makefile")) {
        wsl make clean
    }
    
    Write-Success "Nettoyage terminé."
}

# Installation des dépendances
function Install-Dependencies {
    Write-Status "Installation des dépendances..."
    
    Install-OpenOrbis
    Install-SDL2
    Install-ImGui
    Install-Json
    Install-LibTorrent
    
    Write-Success "Toutes les dépendances ont été installées."
}

# Fonction principale
function Main {
    Write-Host "=========================================="
    Write-Host "    Installation de $ProjectName"
    Write-Host "=========================================="
    Write-Host ""
    
    if ($Help) {
        Show-Help
        return
    }
    
    switch ($Action.ToLower()) {
        "deps" {
            if (Test-Prerequisites) {
                Install-Dependencies
            }
        }
        "build" {
            Build-Project
        }
        "clean" {
            Clean-Build
        }
        "setup-wsl" {
            Setup-WSL
        }
        "help" {
            Show-Help
        }
        "all" {
            if (Test-Prerequisites) {
                Install-Dependencies
                Build-Project
            }
        }
        default {
            Write-Error "Action inconnue: $Action"
            Show-Help
            exit 1
        }
    }
    
    Write-Host ""
    Write-Success "Installation terminée!"
    Write-Host "=========================================="
}

# Gestion des erreurs globales
trap {
    Write-Error "Erreur inattendue: $($_.Exception.Message)"
    Write-Host "Stack trace: $($_.ScriptStackTrace)"
    exit 1
}

# Point d'entrée
if ($MyInvocation.InvocationName -ne '.') {
    Main
}