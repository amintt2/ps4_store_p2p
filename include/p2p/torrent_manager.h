/**
 * PS4 Store P2P - Gestionnaire de Torrents
 * 
 * Gère les téléchargements P2P via libtorrent
 * Permet de télécharger et partager des fichiers .pkg
 */

#ifndef TORRENT_MANAGER_H
#define TORRENT_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

// Forward declarations pour libtorrent
namespace libtorrent {
    class session;
    class torrent_handle;
    class torrent_status;
    class add_torrent_params;
}

// Structure pour les informations de téléchargement
struct DownloadInfo {
    std::string name;
    std::string magnet_link;
    std::string save_path;
    float progress;
    int download_rate;  // bytes/sec
    int upload_rate;    // bytes/sec
    int seeders;
    int leechers;
    int64_t total_size;
    int64_t downloaded;
    bool is_finished;
    bool is_seeding;
    std::string status;
};

// Callbacks pour les événements
using DownloadProgressCallback = std::function<void(const std::string&, float)>;
using DownloadCompleteCallback = std::function<void(const std::string&, const std::string&)>;
using DownloadErrorCallback = std::function<void(const std::string&, const std::string&)>;

class TorrentManager {
public:
    /**
     * Initialise le gestionnaire de torrents
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
     * Démarre le téléchargement d'un torrent
     * @param magnet_link Lien magnet du torrent
     * @param save_path Chemin de sauvegarde
     * @param name Nom du téléchargement
     * @return true en cas de succès
     */
    static bool startDownload(const std::string& magnet_link, 
                             const std::string& save_path,
                             const std::string& name);
    
    /**
     * Arrête un téléchargement
     * @param name Nom du téléchargement
     * @return true en cas de succès
     */
    static bool stopDownload(const std::string& name);
    
    /**
     * Supprime un téléchargement
     * @param name Nom du téléchargement
     * @param delete_files Supprimer aussi les fichiers
     * @return true en cas de succès
     */
    static bool removeDownload(const std::string& name, bool delete_files = false);
    
    /**
     * Partage un fichier .pkg
     * @param pkg_path Chemin vers le fichier .pkg
     * @param name Nom du partage
     * @return true en cas de succès
     */
    static bool sharePackage(const std::string& pkg_path, const std::string& name);
    
    /**
     * Obtient la liste des téléchargements actifs
     * @return Liste des téléchargements
     */
    static std::vector<DownloadInfo> getDownloads();
    
    /**
     * Obtient les informations d'un téléchargement spécifique
     * @param name Nom du téléchargement
     * @return Informations du téléchargement
     */
    static DownloadInfo getDownloadInfo(const std::string& name);
    
    /**
     * Définit le callback de progression
     * @param callback Fonction à appeler lors de la progression
     */
    static void setProgressCallback(DownloadProgressCallback callback);
    
    /**
     * Définit le callback de fin de téléchargement
     * @param callback Fonction à appeler à la fin
     */
    static void setCompleteCallback(DownloadCompleteCallback callback);
    
    /**
     * Définit le callback d'erreur
     * @param callback Fonction à appeler en cas d'erreur
     */
    static void setErrorCallback(DownloadErrorCallback callback);
    
    /**
     * Configure les limites de bande passante
     * @param download_limit Limite de téléchargement (bytes/sec, 0 = illimité)
     * @param upload_limit Limite d'upload (bytes/sec, 0 = illimité)
     */
    static void setBandwidthLimits(int download_limit, int upload_limit);
    
    /**
     * Configure le port d'écoute
     * @param port Port à utiliser (0 = automatique)
     * @return true en cas de succès
     */
    static bool setListenPort(int port);
    
    /**
     * Obtient les statistiques globales
     * @param total_download Téléchargement total (bytes)
     * @param total_upload Upload total (bytes)
     * @param download_rate Vitesse de téléchargement (bytes/sec)
     * @param upload_rate Vitesse d'upload (bytes/sec)
     */
    static void getGlobalStats(int64_t& total_download, int64_t& total_upload,
                              int& download_rate, int& upload_rate);
    
    /**
     * Sauvegarde l'état des torrents
     * @param state_file Fichier de sauvegarde
     * @return true en cas de succès
     */
    static bool saveState(const std::string& state_file);
    
    /**
     * Charge l'état des torrents
     * @param state_file Fichier de sauvegarde
     * @return true en cas de succès
     */
    static bool loadState(const std::string& state_file);

private:
    static std::unique_ptr<libtorrent::session> s_session;
    static std::map<std::string, libtorrent::torrent_handle> s_torrents;
    static std::string s_download_path;
    static std::string s_state_file;
    
    static DownloadProgressCallback s_progress_callback;
    static DownloadCompleteCallback s_complete_callback;
    static DownloadErrorCallback s_error_callback;
    
    // Méthodes internes
    static void processAlerts();
    static void handleTorrentAlert(const libtorrent::torrent_status& status);
    static std::string getStatusString(int state);
    static void createTorrentFile(const std::string& file_path, const std::string& output_path);
};

#endif // TORRENT_MANAGER_H