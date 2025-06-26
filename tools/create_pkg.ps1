<#
.SYNOPSIS
    Script PowerShell pour créer un package PKG PS4

.DESCRIPTION
    Ce script automatise la création d'un package PKG pour PS4 en utilisant
    les outils OpenOrbis et orbis-pub-gen.

.PARAMETER SourceDir
    Répertoire contenant les fichiers à packager

.PARAMETER OutputPkg
    Chemin de sortie du fichier PKG

.PARAMETER ProjectFile
    Fichier de projet GP4 (optionnel)

.EXAMPLE
    .\create_pkg.ps1 -SourceDir "build\pkg" -OutputPkg "build\ps4_store_p2p.pkg"

.NOTES
    Auteur: PS4 Store P2P Team
    Version: 1.0.0
    Nécessite: OpenOrbis Toolchain
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$SourceDir,
    
    [Parameter(Mandatory=$true)]
    [string]$OutputPkg,
    
    [Parameter(Mandatory=$false)]
    [string]$ProjectFile = ""
)

# Configuration
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

# Couleurs pour l'affichage
function Write-ColorOutput {
    param(
        [string]$Message,
        [string]$Color = "White"
    )
    Write-Host $Message -ForegroundColor $Color
}

function Write-Success {
    param([string]$Message)
    Write-ColorOutput "[SUCCESS] $Message" "Green"
}

function Write-Error {
    param([string]$Message)
    Write-ColorOutput "[ERROR] $Message" "Red"
}

function Write-Info {
    param([string]$Message)
    Write-ColorOutput "[INFO] $Message" "Cyan"
}

function Write-Warning {
    param([string]$Message)
    Write-ColorOutput "[WARNING] $Message" "Yellow"
}

# Fonction pour vérifier si un chemin existe
function Test-PathExists {
    param([string]$Path)
    if (-not (Test-Path $Path)) {
        Write-Error "Le chemin '$Path' n'existe pas."
        exit 1
    }
}

# Fonction pour créer un répertoire s'il n'existe pas
function New-DirectoryIfNotExists {
    param([string]$Path)
    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
        Write-Info "Répertoire créé: $Path"
    }
}

# Fonction pour trouver OpenOrbis Toolchain
function Find-OpenOrbisToolchain {
    $possiblePaths = @(
        $env:OPENORBIS_ROOT,
        $env:OO_PS4_TOOLCHAIN,
        "C:\OpenOrbis",
        "C:\opt\OpenOrbis",
        "/opt/OpenOrbis",
        "/usr/local/OpenOrbis"
    )
    
    foreach ($path in $possiblePaths) {
        if ($path -and (Test-Path $path)) {
            Write-Info "OpenOrbis Toolchain trouvé: $path"
            return $path
        }
    }
    
    Write-Error "OpenOrbis Toolchain non trouvé. Définissez OPENORBIS_ROOT."
    exit 1
}

# Fonction pour créer le fichier param.sfo
function New-ParamSfo {
    param(
        [string]$OutputPath,
        [string]$ToolchainPath
    )
    
    $paramSfoPath = Join-Path $OutputPath "sce_sys\param.sfo"
    $sceSystemDir = Join-Path $OutputPath "sce_sys"
    
    New-DirectoryIfNotExists $sceSystemDir
    
    # Contenu du param.sfo (format binaire simulé en texte pour cet exemple)
    $paramContent = @"
APP_TYPE=1
APP_VER=01.00
ATTRIBUTE=0
CATEGORY=gd
CONTENT_ID=IV0000-CUSA00000_00-PS4STOREP2P00000
DOWNLOAD_DATA_SIZE=0
FORMAT=obs
PARENTAL_LEVEL=1
SDK_VER=04508101
SYSTEM_VER=0
TITLE=PS4 Store P2P
TITLE_ID=CUSA00000
VERSION=01.00
"@
    
    # Pour un vrai param.sfo, il faudrait utiliser l'outil approprié
    # Ici on crée un fichier temporaire
    $paramContent | Out-File -FilePath $paramSfoPath -Encoding ASCII
    Write-Info "Fichier param.sfo créé: $paramSfoPath"
}

# Fonction pour créer le fichier GP4
function New-Gp4File {
    param(
        [string]$SourceDir,
        [string]$OutputPath
    )
    
    $gp4Content = @"
<?xml version="1.0" encoding="utf-8"?>
<psproject fmt="gp4" version="1000">
  <volume>
    <volume_type>pkg_ps4_app</volume_type>
    <volume_id>PS4STOREP2P</volume_id>
    <volume_ts>2024-12-01 12:00:00</volume_ts>
    <package content_id="IV0000-CUSA00000_00-PS4STOREP2P00000" passcode="00000000000000000000000000000000" storage_type="digital50" app_type="full">
      <chunk_info chunk_count="1" scenario_count="1">
        <chunks>
          <chunk id="0" layer_no="0" label="Chunk #0"/>
        </chunks>
        <scenarios default_id="0">
          <scenario id="0" type="sp" initial_chunk_count="1" label="Scenario #0">0</scenario>
        </scenarios>
      </chunk_info>
    </package>
  </volume>
  <files img_no="0">
    <file targ_path="sce_sys/param.sfo" orig_path="sce_sys/param.sfo"/>
    <file targ_path="eboot.bin" orig_path="ps4_store_p2p.elf"/>
  </files>
  <rootdir>
    <dir targ_name="sce_sys"/>
  </rootdir>
</psproject>
"@
    
    $gp4Path = Join-Path $OutputPath "project.gp4"
    $gp4Content | Out-File -FilePath $gp4Path -Encoding UTF8
    Write-Info "Fichier GP4 créé: $gp4Path"
    return $gp4Path
}

# Fonction pour convertir ELF en OELF
function Convert-ElfToOelf {
    param(
        [string]$ElfPath,
        [string]$OelfPath,
        [string]$ToolchainPath
    )
    
    $createFselfTool = Join-Path $ToolchainPath "bin\create-fself.exe"
    if (-not (Test-Path $createFselfTool)) {
        $createFselfTool = Join-Path $ToolchainPath "bin\linux\create-fself"
    }
    
    if (-not (Test-Path $createFselfTool)) {
        Write-Warning "create-fself non trouvé, copie directe du fichier ELF"
        Copy-Item $ElfPath $OelfPath
        return
    }
    
    $libPath = Join-Path $ToolchainPath "lib"
    $args = @(
        "-in=$ElfPath",
        "-out=$OelfPath",
        "--lib=$libPath",
        "--sdk-version=0x4508101"
    )
    
    Write-Info "Conversion ELF vers OELF..."
    & $createFselfTool @args
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Conversion ELF vers OELF réussie"
    } else {
        Write-Error "Échec de la conversion ELF vers OELF"
        exit 1
    }
}

# Fonction pour créer le package PKG
function New-PkgFile {
    param(
        [string]$Gp4Path,
        [string]$SourceDir,
        [string]$OutputPkg,
        [string]$ToolchainPath
    )
    
    # Chercher l'outil de création de PKG
    $pkgTools = @(
        Join-Path $ToolchainPath "bin\PkgTool.Core.exe",
        Join-Path $ToolchainPath "bin\linux\PkgTool.Core",
        Join-Path $ToolchainPath "bin\orbis-pub-gen.exe",
        Join-Path $ToolchainPath "bin\linux\orbis-pub-gen"
    )
    
    $pkgTool = $null
    foreach ($tool in $pkgTools) {
        if (Test-Path $tool) {
            $pkgTool = $tool
            break
        }
    }
    
    if (-not $pkgTool) {
        Write-Error "Outil de création PKG non trouvé dans le toolchain"
        exit 1
    }
    
    Write-Info "Utilisation de l'outil: $pkgTool"
    Write-Info "Création du package PKG..."
    
    # Créer le répertoire de sortie
    $outputDir = Split-Path $OutputPkg -Parent
    New-DirectoryIfNotExists $outputDir
    
    # Arguments pour la création du PKG
    if ($pkgTool -like "*PkgTool.Core*") {
        $args = @("pkg_build", $Gp4Path, $outputDir)
    } else {
        $args = @("img_create", $Gp4Path, $OutputPkg)
    }
    
    # Exécuter l'outil
    & $pkgTool @args
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Package PKG créé avec succès: $OutputPkg"
    } else {
        Write-Error "Échec de la création du package PKG"
        exit 1
    }
}

# Fonction pour afficher les informations du package
function Show-PackageInfo {
    param([string]$PkgPath)
    
    if (Test-Path $PkgPath) {
        $fileInfo = Get-Item $PkgPath
        $sizeInMB = [math]::Round($fileInfo.Length / 1MB, 2)
        
        Write-Info "Informations du package:"
        Write-Info "  Fichier: $($fileInfo.Name)"
        Write-Info "  Taille: $sizeInMB MB"
        Write-Info "  Chemin: $($fileInfo.FullName)"
        Write-Info "  Créé le: $($fileInfo.CreationTime)"
    }
}

# Fonction principale
function Main {
    Write-Info "=== Création du Package PKG PS4 ==="
    Write-Info "Source: $SourceDir"
    Write-Info "Sortie: $OutputPkg"
    
    # Vérifications préliminaires
    Test-PathExists $SourceDir
    
    # Trouver OpenOrbis Toolchain
    $toolchainPath = Find-OpenOrbisToolchain
    
    # Créer un répertoire temporaire pour le packaging
    $tempDir = Join-Path $env:TEMP "ps4_pkg_$(Get-Random)"
    New-DirectoryIfNotExists $tempDir
    
    try {
        # Copier les fichiers sources
        Write-Info "Copie des fichiers sources..."
        Copy-Item -Path "$SourceDir\*" -Destination $tempDir -Recurse -Force
        
        # Chercher le fichier ELF
        $elfFiles = Get-ChildItem -Path $tempDir -Filter "*.elf" -Recurse
        if ($elfFiles.Count -eq 0) {
            Write-Error "Aucun fichier ELF trouvé dans $SourceDir"
            exit 1
        }
        
        $elfFile = $elfFiles[0]
        Write-Info "Fichier ELF trouvé: $($elfFile.Name)"
        
        # Renommer le fichier ELF en eboot.bin
        $ebootPath = Join-Path $tempDir "eboot.bin"
        if ($elfFile.FullName -ne $ebootPath) {
            Move-Item $elfFile.FullName $ebootPath
            Write-Info "Fichier ELF renommé en eboot.bin"
        }
        
        # Créer param.sfo
        New-ParamSfo -OutputPath $tempDir -ToolchainPath $toolchainPath
        
        # Créer ou utiliser le fichier GP4
        if ($ProjectFile -and (Test-Path $ProjectFile)) {
            $gp4Path = $ProjectFile
            Write-Info "Utilisation du fichier GP4 existant: $ProjectFile"
        } else {
            $gp4Path = New-Gp4File -SourceDir $tempDir -OutputPath $tempDir
        }
        
        # Créer le package PKG
        New-PkgFile -Gp4Path $gp4Path -SourceDir $tempDir -OutputPkg $OutputPkg -ToolchainPath $toolchainPath
        
        # Afficher les informations du package
        Show-PackageInfo -PkgPath $OutputPkg
        
        Write-Success "Package PKG créé avec succès!"
        
    } catch {
        Write-Error "Erreur lors de la création du package: $($_.Exception.Message)"
        exit 1
    } finally {
        # Nettoyer le répertoire temporaire
        if (Test-Path $tempDir) {
            Remove-Item $tempDir -Recurse -Force -ErrorAction SilentlyContinue
            Write-Info "Répertoire temporaire nettoyé"
        }
    }
}

# Point d'entrée
if ($MyInvocation.InvocationName -ne '.') {
    Main
}