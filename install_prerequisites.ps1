# Script PowerShell pour installer les prérequis manquants
# CMake, Git, Visual Studio Build Tools

param(
    [switch]$Force
)

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

# Vérifier si on est administrateur
function Test-Administrator {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Installer Chocolatey si nécessaire
function Install-Chocolatey {
    if (Get-Command "choco" -ErrorAction SilentlyContinue) {
        Write-Status "Chocolatey déjà installé"
        return $true
    }
    
    Write-Status "Installation de Chocolatey..."
    try {
        Set-ExecutionPolicy Bypass -Scope Process -Force
        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
        Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
        
        # Recharger le PATH
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
        
        Write-Status "Chocolatey installé avec succès"
        return $true
    } catch {
        Write-Error "Erreur lors de l'installation de Chocolatey: $($_.Exception.Message)"
        return $false
    }
}

# Installer CMake
function Install-CMake {
    if (Get-Command "cmake" -ErrorAction SilentlyContinue) {
        Write-Status "CMake déjà installé"
        return $true
    }
    
    Write-Status "Installation de CMake..."
    try {
        choco install cmake -y
        
        # Recharger le PATH
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
        
        # Vérifier l'installation
        if (Get-Command "cmake" -ErrorAction SilentlyContinue) {
            Write-Status "CMake installé avec succès"
            return $true
        } else {
            Write-Warning "CMake installé mais non trouvé dans PATH. Redémarrez PowerShell."
            return $false
        }
    } catch {
        Write-Error "Erreur lors de l'installation de CMake: $($_.Exception.Message)"
        return $false
    }
}

# Installer Git
function Install-Git {
    if (Get-Command "git" -ErrorAction SilentlyContinue) {
        Write-Status "Git déjà installé"
        return $true
    }
    
    Write-Status "Installation de Git..."
    try {
        choco install git -y
        
        # Recharger le PATH
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
        
        Write-Status "Git installé avec succès"
        return $true
    } catch {
        Write-Error "Erreur lors de l'installation de Git: $($_.Exception.Message)"
        return $false
    }
}

# Installer Visual Studio Build Tools
function Install-VSBuildTools {
    # Vérifier si déjà installé
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vs = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vs) {
            Write-Status "Visual Studio Build Tools déjà installé: $vs"
            return $true
        }
    }
    
    Write-Status "Installation de Visual Studio Build Tools..."
    try {
        choco install visualstudio2022buildtools -y --package-parameters "--add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows10SDK.19041"
        
        Write-Status "Visual Studio Build Tools installé avec succès"
        return $true
    } catch {
        Write-Error "Erreur lors de l'installation de Visual Studio Build Tools: $($_.Exception.Message)"
        Write-Warning "Vous pouvez installer manuellement depuis: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022"
        return $false
    }
}

# Script principal
Write-Status "=== Installation des prérequis pour OpenOrbis ==="

# Vérifier les privilèges administrateur
if (-not (Test-Administrator)) {
    Write-Error "Ce script nécessite des privilèges administrateur."
    Write-Status "Relancez PowerShell en tant qu'administrateur et réexécutez ce script."
    exit 1
}

# Installer Chocolatey
if (-not (Install-Chocolatey)) {
    Write-Error "Impossible d'installer Chocolatey. Installation manuelle requise."
    exit 1
}

# Installer les outils
$success = $true

if (-not (Install-Git)) {
    $success = $false
}

if (-not (Install-CMake)) {
    $success = $false
}

if (-not (Install-VSBuildTools)) {
    Write-Warning "Visual Studio Build Tools non installé. Compilation possible avec d'autres outils."
}

if ($success) {
    Write-Status "=== Tous les prérequis sont installés! ==="
    Write-Status "Redémarrez PowerShell et exécutez: .\setup_openorbis.ps1"
} else {
    Write-Error "Certains prérequis n'ont pas pu être installés."
    Write-Status "Installez manuellement:"
    Write-Status "- CMake: https://cmake.org/download/"
    Write-Status "- Git: https://git-scm.com/download/win"
    Write-Status "- Visual Studio Build Tools: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022"
    exit 1
}