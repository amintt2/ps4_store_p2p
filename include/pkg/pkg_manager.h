/**
 * PS4 Store P2P - Gestionnaire de Packages PKG
 * 
 * Gère l'installation et la vérification des fichiers .pkg PS4
 * Interface avec le système de Debug Settings de la PS4
 */

#ifndef PKG_MANAGER_H
#define PKG_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <functional>

// Structure pour les informations d'un package
struct PackageInfo {
    std::string file_path;
    std::string title;
    std::string title_id;
    std::string content_id;
    std::string version;
    std::string category;
    int64_t file_size;
    std::string checksum_sha256;
    bool is_valid;
    bool is_installed;
    std::string install_path;
    std::string icon_path;
    std::string description;
    std::string publisher;
    std::string release_date;
};

// États d'installation
enum class InstallStatus {
    NOT_STARTED,
    COPYING,
    VERIFYING,
    INSTALLING,
    COMPLETED,
    FAILED,
    CANCELLED
};

// Structure pour le suivi d'installation
struct InstallProgress {
    std::string package_name;
    InstallStatus status;
    float progress;  // 0.0 à 1.0
    std::string current_operation;
    std::string error_message;
    int64_t bytes_copied;
    int64_t total_bytes;
};

// Callbacks pour les événements d'installation
using InstallProgressCallback = std::function<void(const InstallProgress&)>;
using InstallCompleteCallback = std::function<void(const std::string&, bool)>;

class PkgManager {
public:
    /**
     * Initialise le gestionnaire de packages
     * @return 0 en cas de succès, -1 en cas d'erreur
     */
    static int initialize();
    
    /**
     * Nettoie les ressources du gestionnaire
     */
    static void cleanup();
    
    /**
     * Met à jour le gestionnaire (à appeler dans la boucle principale)
     */
    static void update();
    
    /**
     * Analyse un fichier .pkg et extrait ses informations
     * @param pkg_path Chemin vers le fichier .pkg
     * @return Informations du package
     */
    static PackageInfo analyzePackage(const std::string& pkg_path);
    
    /**
     * Vérifie l'intégrité d'un fichier .pkg
     * @param pkg_path Chemin vers le fichier .pkg
     * @param expected_checksum Checksum attendu (optionnel)
     * @return true si le fichier est valide
     */
    static bool verifyPackage(const std::string& pkg_path, 
                             const std::string& expected_checksum = "");
    
    /**
     * Installe un package .pkg
     * @param pkg_path Chemin vers le fichier .pkg
     * @param force_install Forcer l'installation même si déjà installé
     * @return true si l'installation a démarré avec succès
     */
    static bool installPackage(const std::string& pkg_path, bool force_install = false);
    
    /**
     * Désinstalle un package
     * @param title_id ID du titre à désinstaller
     * @return true en cas de succès
     */
    static bool uninstallPackage(const std::string& title_id);
    
    /**
     * Obtient la liste des packages installés
     * @return Liste des packages installés
     */
    static std::vector<PackageInfo> getInstalledPackages();
    
    /**
     * Vérifie si un package est installé
     * @param title_id ID du titre
     * @return true si installé
     */
    static bool isPackageInstalled(const std::string& title_id);
    
    /**
     * Obtient les informations d'un package installé
     * @param title_id ID du titre
     * @return Informations du package
     */
    static PackageInfo getInstalledPackageInfo(const std::string& title_id);
    
    /**
     * Obtient le progrès d'installation en cours
     * @return Progrès d'installation
     */
    static InstallProgress getCurrentInstallProgress();
    
    /**
     * Annule l'installation en cours
     * @return true en cas de succès
     */
    static bool cancelCurrentInstall();
    
    /**
     * Copie un fichier .pkg vers le dossier d'installation
     * @param source_path Chemin source
     * @param dest_path Chemin destination
     * @return true en cas de succès
     */
    static bool copyPackageToInstallDir(const std::string& source_path, 
                                       const std::string& dest_path);
    
    /**
     * Lance l'installation via Debug Settings
     * @param pkg_path Chemin vers le fichier .pkg
     * @return true en cas de succès
     */
    static bool triggerDebugInstall(const std::string& pkg_path);
    
    /**
     * Nettoie les fichiers temporaires d'installation
     */
    static void cleanupTempFiles();
    
    /**
     * Définit le callback de progression d'installation
     * @param callback Fonction à appeler
     */
    static void setInstallProgressCallback(InstallProgressCallback callback);
    
    /**
     * Définit le callback de fin d'installation
     * @param callback Fonction à appeler
     */
    static void setInstallCompleteCallback(InstallCompleteCallback callback);
    
    /**
     * Calcule le checksum SHA256 d'un fichier
     * @param file_path Chemin du fichier
     * @return Checksum en hexadécimal
     */
    static std::string calculateSHA256(const std::string& file_path);
    
    /**
     * Obtient l'espace disque disponible
     * @param path Chemin à vérifier
     * @return Espace libre en bytes
     */
    static int64_t getAvailableDiskSpace(const std::string& path);
    
    /**
     * Vérifie si l'espace disque est suffisant pour l'installation
     * @param required_space Espace requis en bytes
     * @return true si suffisant
     */
    static bool checkDiskSpace(int64_t required_space);
    
    /**
     * Obtient le chemin d'installation par défaut
     * @return Chemin d'installation
     */
    static std::string getDefaultInstallPath();
    
    /**
     * Définit le chemin d'installation
     * @param path Nouveau chemin
     */
    static void setInstallPath(const std::string& path);

private:
    static std::string s_install_path;
    static std::string s_temp_path;
    static InstallProgress s_current_install;
    static bool s_install_in_progress;
    
    static InstallProgressCallback s_progress_callback;
    static InstallCompleteCallback s_complete_callback;
    
    // Méthodes internes
    static bool parsePkgHeader(const std::string& pkg_path, PackageInfo& info);
    static bool extractPkgMetadata(const std::string& pkg_path, PackageInfo& info);
    static bool validatePkgStructure(const std::string& pkg_path);
    static void updateInstallProgress(const std::string& operation, float progress);
    static bool copyFileWithProgress(const std::string& source, const std::string& dest);
    static std::string formatFileSize(int64_t bytes);
    static bool createDirectoryRecursive(const std::string& path);
};

#endif // PKG_MANAGER_H