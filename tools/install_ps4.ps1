<#
.SYNOPSIS
    Script PowerShell pour installer un package PKG sur PS4 via FTP

.DESCRIPTION
    Ce script automatise l'installation d'un package PKG sur une PS4 jailbreakée
    en utilisant FTP et les Debug Settings.

.PARAMETER PkgPath
    Chemin vers le fichier PKG à installer

.PARAMETER PS4IP
    Adresse IP de la PS4 (par défaut: 192.168.1.100)

.PARAMETER FTPPort
    Port FTP de la PS4 (par défaut: 1337)

.PARAMETER Username
    Nom d'utilisateur FTP (par défaut: anonymous)

.PARAMETER Password
    Mot de passe FTP (par défaut: anonymous)

.PARAMETER InstallPath
    Chemin d'installation sur la PS4 (par défaut: /data/pkg)

.EXAMPLE
    .\install_ps4.ps1 -PkgPath "build\ps4_store_p2p.pkg" -PS4IP "192.168.1.100"

.NOTES
    Auteur: PS4 Store P2P Team
    Version: 1.0.0
    Nécessite: PS4 jailbreakée avec FTP activé
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$PkgPath,
    
    [Parameter(Mandatory=$false)]
    [string]$PS4IP = "192.168.1.100",
    
    [Parameter(Mandatory=$false)]
    [int]$FTPPort = 1337,
    
    [Parameter(Mandatory=$false)]
    [string]$Username = "anonymous",
    
    [Parameter(Mandatory=$false)]
    [string]$Password = "anonymous",
    
    [Parameter(Mandatory=$false)]
    [string]$InstallPath = "/data/pkg"
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

# Fonction pour tester la connectivité réseau
function Test-PS4Connection {
    param(
        [string]$IPAddress,
        [int]$Port
    )
    
    Write-Info "Test de connexion vers $IPAddress:$Port..."
    
    try {
        $tcpClient = New-Object System.Net.Sockets.TcpClient
        $tcpClient.ReceiveTimeout = 5000
        $tcpClient.SendTimeout = 5000
        
        $result = $tcpClient.BeginConnect($IPAddress, $Port, $null, $null)
        $success = $result.AsyncWaitHandle.WaitOne(5000, $true)
        
        if ($success -and $tcpClient.Connected) {
            $tcpClient.Close()
            Write-Success "Connexion réussie vers $IPAddress:$Port"
            return $true
        } else {
            $tcpClient.Close()
            Write-Error "Impossible de se connecter à $IPAddress:$Port"
            return $false
        }
    } catch {
        Write-Error "Erreur de connexion: $($_.Exception.Message)"
        return $false
    }
}

# Fonction pour créer une session FTP
function New-FTPSession {
    param(
        [string]$Server,
        [int]$Port,
        [string]$Username,
        [string]$Password
    )
    
    $ftpUri = "ftp://$Server:$Port"
    
    try {
        # Test de connexion FTP
        $request = [System.Net.FtpWebRequest]::Create($ftpUri)
        $request.Method = [System.Net.WebRequestMethods+Ftp]::ListDirectory
        $request.Credentials = New-Object System.Net.NetworkCredential($Username, $Password)
        $request.Timeout = 10000
        $request.UseBinary = $true
        $request.UsePassive = $true
        
        $response = $request.GetResponse()
        $response.Close()
        
        Write-Success "Session FTP établie avec $Server:$Port"
        return @{
            Server = $Server
            Port = $Port
            Username = $Username
            Password = $Password
            BaseUri = $ftpUri
        }
    } catch {
        Write-Error "Échec de la connexion FTP: $($_.Exception.Message)"
        throw
    }
}

# Fonction pour créer un répertoire FTP
function New-FTPDirectory {
    param(
        [hashtable]$Session,
        [string]$DirectoryPath
    )
    
    $ftpUri = "$($Session.BaseUri)$DirectoryPath"
    
    try {
        $request = [System.Net.FtpWebRequest]::Create($ftpUri)
        $request.Method = [System.Net.WebRequestMethods+Ftp]::MakeDirectory
        $request.Credentials = New-Object System.Net.NetworkCredential($Session.Username, $Session.Password)
        $request.Timeout = 10000
        
        $response = $request.GetResponse()
        $response.Close()
        
        Write-Info "Répertoire créé: $DirectoryPath"
    } catch {
        # Le répertoire existe peut-être déjà
        if ($_.Exception.Message -notlike "*550*") {
            Write-Warning "Erreur lors de la création du répertoire: $($_.Exception.Message)"
        }
    }
}

# Fonction pour uploader un fichier via FTP
function Send-FTPFile {
    param(
        [hashtable]$Session,
        [string]$LocalPath,
        [string]$RemotePath
    )
    
    if (-not (Test-Path $LocalPath)) {
        Write-Error "Fichier local non trouvé: $LocalPath"
        return $false
    }
    
    $fileInfo = Get-Item $LocalPath
    $fileSizeMB = [math]::Round($fileInfo.Length / 1MB, 2)
    
    Write-Info "Upload de $($fileInfo.Name) ($fileSizeMB MB)..."
    
    $ftpUri = "$($Session.BaseUri)$RemotePath"
    
    try {
        $request = [System.Net.FtpWebRequest]::Create($ftpUri)
        $request.Method = [System.Net.WebRequestMethods+Ftp]::UploadFile
        $request.Credentials = New-Object System.Net.NetworkCredential($Session.Username, $Session.Password)
        $request.UseBinary = $true
        $request.UsePassive = $true
        $request.Timeout = 300000  # 5 minutes
        $request.ContentLength = $fileInfo.Length
        
        # Lire le fichier et l'envoyer
        $fileStream = [System.IO.File]::OpenRead($LocalPath)
        $ftpStream = $request.GetRequestStream()
        
        $buffer = New-Object byte[] 8192
        $totalBytes = 0
        $lastProgress = 0
        
        while (($bytesRead = $fileStream.Read($buffer, 0, $buffer.Length)) -gt 0) {
            $ftpStream.Write($buffer, 0, $bytesRead)
            $totalBytes += $bytesRead
            
            # Afficher le progrès
            $progress = [math]::Round(($totalBytes / $fileInfo.Length) * 100, 1)
            if ($progress -ge $lastProgress + 10) {
                Write-Info "Progrès: $progress% ($([math]::Round($totalBytes / 1MB, 1)) MB / $fileSizeMB MB)"
                $lastProgress = $progress
            }
        }
        
        $fileStream.Close()
        $ftpStream.Close()
        
        $response = $request.GetResponse()
        $response.Close()
        
        Write-Success "Fichier uploadé avec succès: $RemotePath"
        return $true
        
    } catch {
        Write-Error "Échec de l'upload: $($_.Exception.Message)"
        return $false
    }
}

# Fonction pour envoyer une commande HTTP à la PS4
function Send-PS4Command {
    param(
        [string]$PS4IP,
        [string]$Command,
        [hashtable]$Parameters = @{}
    )
    
    $baseUrl = "http://$PS4IP:12800"
    
    try {
        $uri = "$baseUrl/$Command"
        
        if ($Parameters.Count -gt 0) {
            $queryString = ($Parameters.GetEnumerator() | ForEach-Object { "$($_.Key)=$($_.Value)" }) -join "&"
            $uri += "?$queryString"
        }
        
        Write-Info "Envoi de la commande: $Command"
        
        $response = Invoke-WebRequest -Uri $uri -Method GET -TimeoutSec 30
        
        if ($response.StatusCode -eq 200) {
            Write-Success "Commande exécutée avec succès"
            return $true
        } else {
            Write-Warning "Réponse inattendue: $($response.StatusCode)"
            return $false
        }
        
    } catch {
        Write-Warning "Impossible d'envoyer la commande HTTP: $($_.Exception.Message)"
        return $false
    }
}

# Fonction pour installer le package via Debug Settings
function Install-PS4Package {
    param(
        [string]$PS4IP,
        [string]$PackagePath
    )
    
    Write-Info "Installation du package via Debug Settings..."
    
    # Essayer d'installer via l'API HTTP (si disponible)
    $success = Send-PS4Command -PS4IP $PS4IP -Command "install" -Parameters @{ "path" = $PackagePath }
    
    if ($success) {
        Write-Success "Installation lancée via l'API HTTP"
    } else {
        Write-Info "Installation manuelle requise:"
        Write-Info "1. Allez dans Paramètres > Système > Console Info"
        Write-Info "2. Appuyez sur les boutons dans l'ordre: L1+L2+R1+R2+Options"
        Write-Info "3. Allez dans Debug Settings > Game > Package Installer"
        Write-Info "4. Sélectionnez le fichier: $PackagePath"
        Write-Info "5. Confirmez l'installation"
    }
}

# Fonction pour afficher les instructions post-installation
function Show-PostInstallInstructions {
    Write-Info "=== Instructions post-installation ==="
    Write-Info "1. L'application devrait apparaître dans le menu principal de la PS4"
    Write-Info "2. Si l'icône n'apparaît pas, redémarrez la PS4"
    Write-Info "3. Lancez l'application depuis le menu des jeux"
    Write-Info "4. Configurez les paramètres réseau dans l'application"
    Write-Info "5. Profitez du store P2P!"
    Write-Info ""
    Write-Warning "Note: Cette application nécessite une PS4 jailbreakée avec GoldHEN"
}

# Fonction principale
function Main {
    Write-Info "=== Installation PS4 Store P2P ==="
    Write-Info "Package: $PkgPath"
    Write-Info "PS4 IP: $PS4IP:$FTPPort"
    Write-Info "Chemin d'installation: $InstallPath"
    
    # Vérifier que le fichier PKG existe
    if (-not (Test-Path $PkgPath)) {
        Write-Error "Fichier PKG non trouvé: $PkgPath"
        exit 1
    }
    
    $pkgFile = Get-Item $PkgPath
    $remotePkgPath = "$InstallPath/$($pkgFile.Name)"
    
    try {
        # Tester la connexion réseau
        if (-not (Test-PS4Connection -IPAddress $PS4IP -Port $FTPPort)) {
            Write-Error "Impossible de se connecter à la PS4. Vérifiez:"
            Write-Error "  - L'adresse IP de la PS4: $PS4IP"
            Write-Error "  - Le serveur FTP est activé sur la PS4"
            Write-Error "  - Le port FTP: $FTPPort"
            Write-Error "  - La PS4 et le PC sont sur le même réseau"
            exit 1
        }
        
        # Créer la session FTP
        Write-Info "Connexion FTP à la PS4..."
        $ftpSession = New-FTPSession -Server $PS4IP -Port $FTPPort -Username $Username -Password $Password
        
        # Créer le répertoire de destination
        New-FTPDirectory -Session $ftpSession -DirectoryPath $InstallPath
        
        # Uploader le fichier PKG
        Write-Info "Upload du package PKG..."
        $uploadSuccess = Send-FTPFile -Session $ftpSession -LocalPath $PkgPath -RemotePath $remotePkgPath
        
        if (-not $uploadSuccess) {
            Write-Error "Échec de l'upload du package"
            exit 1
        }
        
        # Installer le package
        Install-PS4Package -PS4IP $PS4IP -PackagePath $remotePkgPath
        
        # Afficher les instructions
        Show-PostInstallInstructions
        
        Write-Success "Installation terminée avec succès!"
        
    } catch {
        Write-Error "Erreur lors de l'installation: $($_.Exception.Message)"
        exit 1
    }
}

# Point d'entrée
if ($MyInvocation.InvocationName -ne '.') {
    Main
}