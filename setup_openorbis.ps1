# Script PowerShell pour installer et compiler OpenOrbis PS4 Toolchain
# Puis compiler le projet PS4 Store P2P

param(
    [switch]$SkipOpenOrbis,
    [switch]$CompileOnly
)

# Configuration
$ProjectRoot = $PSScriptRoot
$OpenOrbisPath = Join-Path $ProjectRoot "OpenOrbis-PS4-Toolchain"
$OpenOrbisUrl = "https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain.git"

# Fonction pour afficher les messages
function Write-Status {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "[WARN] $Message" -ForegroundColor Yellow
}

function Write-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

# Vérifier les prérequis
function Test-Prerequisites {
    Write-Status "Vérification des prérequis..."
    
    $missingTools = @()
    
    # Vérifier Git
    try {
        git --version | Out-Null
        Write-Status "Git trouvé"
    } catch {
        Write-Warning "Git non trouvé"
        $missingTools += "Git"
    }
    
    # Vérifier CMake
    try {
        cmake --version | Out-Null
        Write-Status "CMake trouvé"
    } catch {
        Write-Warning "CMake non trouvé"
        $missingTools += "CMake"
    }
    
    # Vérifier Visual Studio Build Tools ou Visual Studio
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vs = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vs) {
            Write-Status "Visual Studio Build Tools trouvé: $vs"
        } else {
            Write-Warning "Visual Studio Build Tools non trouvé"
            $missingTools += "Visual Studio Build Tools"
        }
    } else {
        Write-Warning "Visual Studio Build Tools non trouvé"
        $missingTools += "Visual Studio Build Tools"
    }
    
    if ($missingTools.Count -gt 0) {
        Write-Warning "Outils manquants: $($missingTools -join ', ')"
        Write-Status "Voulez-vous installer automatiquement les prérequis manquants? (O/N)"
        $response = Read-Host
        if ($response -eq 'O' -or $response -eq 'o' -or $response -eq 'Y' -or $response -eq 'y') {
            Write-Status "Lancement de l'installation des prérequis..."
            Write-Status "Exécutez d'abord: .\install_prerequisites.ps1 (en tant qu'administrateur)"
            Write-Status "Puis relancez ce script."
            return $false
        } else {
            Write-Error "Prérequis manquants. Installation impossible."
            return $false
        }
    }
    
    return $true
}

# Installer OpenOrbis Toolchain
function Install-OpenOrbis {
    Write-Status "Installation d'OpenOrbis Toolchain..."
    
    # Supprimer l'ancien répertoire s'il existe
    if (Test-Path $OpenOrbisPath) {
        Write-Status "Suppression de l'ancien répertoire OpenOrbis..."
        Remove-Item $OpenOrbisPath -Recurse -Force
    }
    
    # Télécharger la dernière release pré-compilée
    try {
        # Obtenir l'URL de la dernière release
        $releaseInfo = Invoke-RestMethod -Uri "https://api.github.com/repos/OpenOrbis/OpenOrbis-PS4-Toolchain/releases/latest"
        $windowsAsset = $releaseInfo.assets | Where-Object { $_.name -like "*toolchain*" -and $_.name -like "*.zip" } | Select-Object -First 1
        
        if (-not $windowsAsset) {
            throw "Aucune release Windows trouvée"
        }
        
        $downloadUrl = $windowsAsset.browser_download_url
        $zipPath = Join-Path $ProjectRoot "toolchain-llvm-12.zip"
        
        # Vérifier si le fichier existe déjà
        if (-not (Test-Path $zipPath)) {
            Write-Status "Téléchargement de la dernière release OpenOrbis..."
            Write-Status "Téléchargement depuis: $downloadUrl"
            Invoke-WebRequest -Uri $downloadUrl -OutFile $zipPath
        } else {
            Write-Status "Utilisation du fichier OpenOrbis mis en cache: $zipPath"
        }
        
        # Extraire l'archive
        Write-Status "Extraction de l'archive..."
        Expand-Archive -Path $zipPath -DestinationPath $OpenOrbisPath -Force
        
        # Note: Le fichier zip est conservé pour éviter les téléchargements futurs
        
        Write-Status "OpenOrbis Toolchain installé avec succès!"
        return $true
        
    } catch {
        Write-Error "Erreur lors du téléchargement d'OpenOrbis: $($_.Exception.Message)"
        Write-Status "Tentative de clonage du repository comme solution de secours..."
        
        try {
            git clone $OpenOrbisUrl $OpenOrbisPath
            if (-not $?) {
                throw "Erreur lors du clonage"
            }
            Write-Warning "Repository cloné, mais vous devrez compiler manuellement"
            return $true
        } catch {
            Write-Error "Erreur lors du clonage d'OpenOrbis: $($_.Exception.Message)"
            return $false
        }
    }
}

# Compiler OpenOrbis avec Visual Studio
function Build-OpenOrbisVS {
    Write-Status "Compilation d'OpenOrbis avec Visual Studio..."
    
    $buildDir = Join-Path $OpenOrbisPath "build"
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    }
    
    Push-Location $buildDir
    try {
        # Vérifier que CMake est disponible
        try {
            cmake --version | Out-Null
        } catch {
            throw "CMake non trouvé. Exécutez d'abord: .\install_prerequisites.ps1"
        }
        
        # Configuration avec CMake
        Write-Status "Configuration CMake..."
        
        # Essayer différents générateurs
        $generators = @(
            "Visual Studio 17 2022",
            "Visual Studio 16 2019", 
            "Visual Studio 15 2017",
            "NMake Makefiles",
            "MinGW Makefiles",
            "Unix Makefiles"
        )
        
        $configSuccess = $false
        foreach ($generator in $generators) {
            Write-Status "Tentative avec générateur: $generator"
            try {
                if ($generator -like "Visual Studio*") {
                    cmake .. -G $generator -A x64
                } else {
                    cmake .. -G $generator
                }
                if ($?) {
                    $configSuccess = $true
                    Write-Status "Configuration réussie avec: $generator"
                    break
                }
            } catch {
                Write-Warning "Échec avec $generator : $($_.Exception.Message)"
                continue
            }
        }
        
        if (-not $configSuccess) {
            throw "Aucun générateur CMake fonctionnel trouvé"
        }
        
        # Compilation
        Write-Status "Compilation en cours..."
        cmake --build . --config Release
        if (-not $?) {
            # Tentative avec make si disponible
            if (Get-Command "make" -ErrorAction SilentlyContinue) {
                Write-Status "Tentative avec make..."
                make -j4
                if (-not $?) {
                    throw "Erreur de compilation avec make"
                }
            } else {
                throw "Erreur de compilation"
            }
        }
        
        Write-Status "OpenOrbis compilé avec succès!"
        return $true
    } catch {
        Write-Error "Erreur lors de la compilation d'OpenOrbis: $($_.Exception.Message)"
        Write-Status "Solutions possibles:"
        Write-Status "1. Exécutez: .\install_prerequisites.ps1 (en tant qu'administrateur)"
        Write-Status "2. Installez manuellement CMake, Git et Visual Studio Build Tools"
        Write-Status "3. Redémarrez PowerShell après installation"
        return $false
    } finally {
        Pop-Location
    }
}

# Compiler le projet PS4
function Build-PS4Project {
    Write-Status "Compilation du projet PS4 Store P2P..."
    
    # Définir les variables d'environnement
    $env:OPENORBIS_ROOT = $OpenOrbisPath
    $env:OO_PS4_TOOLCHAIN = $OpenOrbisPath
    
    Push-Location $ProjectRoot
    try {
        # Utiliser le Makefile avec MinGW ou MSYS2 si disponible
        if (Get-Command "make" -ErrorAction SilentlyContinue) {
            Write-Status "Compilation avec Make..."
            make ps4
            if ($?) {
                Write-Status "Projet PS4 compilé avec succès!"
                return $true
            }
        }
        
        # Alternative: utiliser CMake directement
        Write-Status "Compilation avec CMake..."
        $buildDir = Join-Path $ProjectRoot "build"
        if (-not (Test-Path $buildDir)) {
            New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
        }
        
        cmake -B $buildDir -S . -DCMAKE_BUILD_TYPE=Release -DOPENORBIS_ROOT=$OpenOrbisPath -DPS4_BUILD=ON
        cmake --build $buildDir --config Release
        
        if ($?) {
            Write-Status "Projet PS4 compilé avec succès!"
            return $true
        } else {
            throw "Erreur de compilation"
        }
    } catch {
        Write-Error "Erreur lors de la compilation du projet: $($_.Exception.Message)"
        return $false
    } finally {
        Pop-Location
    }
}

# Script principal
Write-Status "=== Setup OpenOrbis PS4 Toolchain ==="

# Vérifier les prérequis
if (-not (Test-Prerequisites)) {
    Write-Error "Prérequis manquants. Installation interrompue."
    exit 1
}

# Installer OpenOrbis si nécessaire
if (-not $SkipOpenOrbis -and -not $CompileOnly) {
    if (-not (Install-OpenOrbis)) {
        Write-Error "Erreur lors de l'installation d'OpenOrbis"
        exit 1
    }
    
    # Définir la variable d'environnement OO_PS4_TOOLCHAIN
    Write-Status "Configuration de la variable d'environnement OO_PS4_TOOLCHAIN..."
    [Environment]::SetEnvironmentVariable("OO_PS4_TOOLCHAIN", $OpenOrbisPath, "User")
    $env:OO_PS4_TOOLCHAIN = $OpenOrbisPath
    
    Write-Status "OpenOrbis Toolchain configuré avec succès!"
}

# Compiler le projet PS4
if (-not (Build-PS4Project)) {
    Write-Error "Erreur lors de la compilation du projet PS4"
    exit 1
}

Write-Status "=== Installation et compilation terminées avec succès! ==="
Write-Status "Vous pouvez maintenant créer le PKG avec: .\tools\create_pkg.ps1"
Write-Status "Et l'installer sur PS4 avec: .\tools\install_ps4.ps1"