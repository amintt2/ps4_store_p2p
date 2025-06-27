/**
 * PS4 Store P2P - Implémentation du Gestionnaire de Torrents
 */

#include "p2p/torrent_manager.h"
#include "utils/utils.h"

#ifndef NO_LIBTORRENT
#include <libtorrent/session.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/file_storage.hpp>
#include <libtorrent/bencode.hpp>
#endif

#include <fstream>
#include <iostream>
#include <thread>

// Variables statiques
#ifndef NO_LIBTORRENT
std::unique_ptr<libtorrent::session> TorrentManager::s_session = nullptr;
std::map<std::string, libtorrent::torrent_handle> TorrentManager::s_torrents;
#endif
std::string TorrentManager::s_download_path = "/data/ps4_store/downloads";
std::string TorrentManager::s_state_file = "/data/ps4_store/session.state";

DownloadProgressCallback TorrentManager::s_progress_callback = nullptr;
DownloadCompleteCallback TorrentManager::s_complete_callback = nullptr;
DownloadErrorCallback TorrentManager::s_error_callback = nullptr;

int TorrentManager::initialize() {
    LOG_INFO("Initialisation du gestionnaire de torrents...");
    
#ifndef NO_LIBTORRENT
    try {
        // Création de la session libtorrent
        libtorrent::settings_pack settings;
        
        // Configuration de base
        settings.set_str(libtorrent::settings_pack::listen_interfaces, "0.0.0.0:6881");
        settings.set_bool(libtorrent::settings_pack::enable_dht, true);
        settings.set_bool(libtorrent::settings_pack::enable_lsd, true);
        settings.set_bool(libtorrent::settings_pack::enable_upnp, true);
        settings.set_bool(libtorrent::settings_pack::enable_natpmp, true);
        
        // Optimisations pour PS4
        settings.set_int(libtorrent::settings_pack::alert_mask, 
                        libtorrent::alert::error_notification |
                        libtorrent::alert::storage_notification |
                        libtorrent::alert::tracker_notification |
                        libtorrent::alert::status_notification);
        
        // Limites de connexions adaptées à la PS4
        settings.set_int(libtorrent::settings_pack::connections_limit, 50);
        settings.set_int(libtorrent::settings_pack::unchoke_slots_limit, 8);
        
        // Création de la session
        s_session = std::make_unique<libtorrent::session>(settings);
        
        // Création du dossier de téléchargement
        if (!Utils::directoryExists(s_download_path)) {
            Utils::createDirectory(s_download_path);
        }
        
        // Chargement de l'état précédent si disponible
        loadState(s_state_file);
        
        LOG_INFO("Gestionnaire de torrents initialisé avec succès");
        return 0;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de l'initialisation du gestionnaire de torrents: " + std::string(e.what()));
        return -1;
    }
#else
    LOG_INFO("Gestionnaire de torrents désactivé (mode développement)");
    return 0;
#endif
}

void TorrentManager::cleanup() {
    LOG_INFO("Nettoyage du gestionnaire de torrents...");
    
#ifndef NO_LIBTORRENT
    if (s_session) {
        // Sauvegarde de l'état
        saveState(s_state_file);
        
        // Arrêt de tous les torrents
        for (auto& pair : s_torrents) {
            pair.second.pause();
        }
        
        // Attente de l'arrêt propre
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        s_torrents.clear();
        s_session.reset();
    }
#endif
    
    LOG_INFO("Gestionnaire de torrents nettoyé");
}

void TorrentManager::update() {
#ifndef NO_LIBTORRENT
    if (!s_session) return;
    
    // Traitement des alertes
    processAlerts();
#endif
}

bool TorrentManager::startDownload(const std::string& magnet_link, 
                                  const std::string& save_path,
                                  const std::string& name) {
#ifndef NO_LIBTORRENT
    if (!s_session) {
        LOG_ERROR("Session non initialisée");
        return false;
    }
    
    try {
        LOG_INFO("Démarrage du téléchargement: " + name);
        
        // Parsing du lien magnet
        libtorrent::add_torrent_params params;
        libtorrent::error_code ec;
        libtorrent::parse_magnet_uri(magnet_link, params, ec);
        
        if (ec) {
            LOG_ERROR("Erreur lors du parsing du lien magnet: " + ec.message());
            return false;
        }
        
        // Configuration du téléchargement
        params.save_path = save_path.empty() ? s_download_path : save_path;
        params.flags |= libtorrent::torrent_flags::auto_managed;
        params.flags |= libtorrent::torrent_flags::duplicate_is_error;
        
        // Ajout du torrent à la session
        libtorrent::torrent_handle handle = s_session->add_torrent(params, ec);
        
        if (ec) {
            LOG_ERROR("Erreur lors de l'ajout du torrent: " + ec.message());
            return false;
        }
        
        // Stockage du handle
        s_torrents[name] = handle;
        
        LOG_INFO("Téléchargement démarré avec succès: " + name);
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception lors du démarrage du téléchargement: " + std::string(e.what()));
        return false;
    }
}

bool TorrentManager::stopDownload(const std::string& name) {
#ifndef NO_LIBTORRENT
    auto it = s_torrents.find(name);
    if (it == s_torrents.end()) {
        LOG_WARNING("Téléchargement non trouvé: " + name);
        return false;
    }
    
    try {
        it->second.pause();
        LOG_INFO("Téléchargement arrêté: " + name);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de l'arrêt du téléchargement: " + std::string(e.what()));
        return false;
    }
#else
    LOG_WARNING("Téléchargement P2P non disponible (mode développement)");
    return false;
#endif
}

bool TorrentManager::removeDownload(const std::string& name, bool delete_files) {
#ifndef NO_LIBTORRENT
    auto it = s_torrents.find(name);
    if (it == s_torrents.end()) {
        LOG_WARNING("Téléchargement non trouvé: " + name);
        return false;
    }
    
    try {
        libtorrent::remove_flags_t flags = libtorrent::session::delete_partfile;
        if (delete_files) {
            flags |= libtorrent::session::delete_files;
        }
        
        s_session->remove_torrent(it->second, flags);
        s_torrents.erase(it);
        
        LOG_INFO("Téléchargement supprimé: " + name);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de la suppression du téléchargement: " + std::string(e.what()));
        return false;
    }
#else
    LOG_WARNING("Téléchargement P2P non disponible (mode développement)");
    return false;
#endif
}

bool TorrentManager::sharePackage(const std::string& pkg_path, const std::string& name) {
#ifndef NO_LIBTORRENT
    if (!Utils::fileExists(pkg_path)) {
        LOG_ERROR("Fichier PKG non trouvé: " + pkg_path);
        return false;
    }
    
    try {
        LOG_INFO("Création du torrent pour: " + name);
        
        // Création du fichier torrent
        std::string torrent_path = s_download_path + "/" + name + ".torrent";
        createTorrentFile(pkg_path, torrent_path);
        
        // Ajout du torrent en mode seed
        libtorrent::add_torrent_params params;
        params.ti = std::make_shared<libtorrent::torrent_info>(torrent_path);
        params.save_path = Utils::directoryExists(pkg_path) ? pkg_path : 
                          pkg_path.substr(0, pkg_path.find_last_of('/'));
        params.flags |= libtorrent::torrent_flags::seed_mode;
        params.flags |= libtorrent::torrent_flags::auto_managed;
        
        libtorrent::error_code ec;
        libtorrent::torrent_handle handle = s_session->add_torrent(params, ec);
        
        if (ec) {
            LOG_ERROR("Erreur lors de l'ajout du torrent en seed: " + ec.message());
            return false;
        }
        
        s_torrents[name] = handle;
        
        LOG_INFO("Package partagé avec succès: " + name);
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors du partage du package: " + std::string(e.what()));
        return false;
    }
#else
    LOG_WARNING("Partage P2P non disponible (mode développement)");
    return false;
#endif
}

std::vector<DownloadInfo> TorrentManager::getDownloads() {
    std::vector<DownloadInfo> downloads;
    
#ifndef NO_LIBTORRENT
    for (const auto& pair : s_torrents) {
        DownloadInfo info = getDownloadInfo(pair.first);
        downloads.push_back(info);
    }
#endif
    
    return downloads;
}

DownloadInfo TorrentManager::getDownloadInfo(const std::string& name) {
    DownloadInfo info;
    info.name = name;
    
#ifndef NO_LIBTORRENT
    auto it = s_torrents.find(name);
    if (it == s_torrents.end()) {
        info.status = "Non trouvé";
        return info;
    }
    
    try {
        libtorrent::torrent_status status = it->second.status();
        
        info.progress = status.progress;
        info.download_rate = status.download_rate;
        info.upload_rate = status.upload_rate;
        info.seeders = status.num_seeds;
        info.leechers = status.num_peers - status.num_seeds;
        info.total_size = status.total_wanted;
        info.downloaded = status.total_wanted_done;
        info.is_finished = status.is_finished;
        info.is_seeding = status.is_seeding;
        info.status = getStatusString(status.state);
        info.save_path = status.save_path;
        
        // Génération du lien magnet si disponible
        if (status.has_metadata) {
            info.magnet_link = libtorrent::make_magnet_uri(it->second);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de la récupération des infos: " + std::string(e.what()));
        info.status = "Erreur";
    }
#else
    info.status = "P2P non disponible";
#endif
    
    return info;
}

void TorrentManager::setProgressCallback(DownloadProgressCallback callback) {
    s_progress_callback = callback;
}

void TorrentManager::setCompleteCallback(DownloadCompleteCallback callback) {
    s_complete_callback = callback;
}

void TorrentManager::setErrorCallback(DownloadErrorCallback callback) {
    s_error_callback = callback;
}

void TorrentManager::setBandwidthLimits(int download_limit, int upload_limit) {
#ifndef NO_LIBTORRENT
    if (!s_session) return;
    
    libtorrent::settings_pack settings;
    settings.set_int(libtorrent::settings_pack::download_rate_limit, download_limit);
    settings.set_int(libtorrent::settings_pack::upload_rate_limit, upload_limit);
    s_session->apply_settings(settings);
    
    LOG_INFO("Limites de bande passante définies - DL: " + std::to_string(download_limit) + 
             " UL: " + std::to_string(upload_limit));
#endif
}

bool TorrentManager::setListenPort(int port) {
#ifndef NO_LIBTORRENT
    if (!s_session) return false;
    
    try {
        libtorrent::settings_pack settings;
        settings.set_str(libtorrent::settings_pack::listen_interfaces, 
                        "0.0.0.0:" + std::to_string(port));
        s_session->apply_settings(settings);
        
        LOG_INFO("Port d'écoute défini: " + std::to_string(port));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de la définition du port: " + std::string(e.what()));
        return false;
    }
#else
    return false;
#endif
}

void TorrentManager::getGlobalStats(int64_t& total_download, int64_t& total_upload,
                                   int& download_rate, int& upload_rate) {
#ifndef NO_LIBTORRENT
    if (!s_session) {
        total_download = total_upload = download_rate = upload_rate = 0;
        return;
    }
    
    libtorrent::session_status stats = s_session->status();
    total_download = stats.total_download;
    total_upload = stats.total_upload;
    download_rate = stats.download_rate;
    upload_rate = stats.upload_rate;
#else
    total_download = total_upload = download_rate = upload_rate = 0;
#endif
}

bool TorrentManager::saveState(const std::string& state_file) {
#ifndef NO_LIBTORRENT
    if (!s_session) return false;
    
    try {
        std::vector<char> state_data;
        libtorrent::bencode(std::back_inserter(state_data), s_session->save_state());
        
        std::ofstream file(state_file, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERROR("Impossible d'ouvrir le fichier d'état: " + state_file);
            return false;
        }
        
        file.write(state_data.data(), state_data.size());
        file.close();
        
        LOG_DEBUG("État de session sauvegardé");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de la sauvegarde de l'état: " + std::string(e.what()));
        return false;
    }
#else
    return false;
#endif
}

bool TorrentManager::loadState(const std::string& state_file) {
#ifndef NO_LIBTORRENT
    if (!s_session || !Utils::fileExists(state_file)) return false;
    
    try {
        std::ifstream file(state_file, std::ios::binary);
        if (!file.is_open()) return false;
        
        std::vector<char> state_data((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
        file.close();
        
        libtorrent::bdecode_node state;
        libtorrent::error_code ec;
        libtorrent::bdecode(state_data.data(), state_data.data() + state_data.size(), state, ec);
        
        if (!ec) {
            s_session->load_state(state);
            LOG_DEBUG("État de session chargé");
            return true;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors du chargement de l'état: " + std::string(e.what()));
    }
#endif
    
    return false;
}

// Méthodes privées
void TorrentManager::processAlerts() {
#ifndef NO_LIBTORRENT
    if (!s_session) return;
    
    std::vector<libtorrent::alert*> alerts;
    s_session->pop_alerts(&alerts);
    
    for (libtorrent::alert* alert : alerts) {
        switch (alert->type()) {
            case libtorrent::torrent_finished_alert::alert_type: {
                auto* finished_alert = libtorrent::alert_cast<libtorrent::torrent_finished_alert>(alert);
                if (finished_alert && s_complete_callback) {
                    std::string name = finished_alert->handle.status().name;
                    std::string save_path = finished_alert->handle.status().save_path;
                    s_complete_callback(name, save_path);
                }
                break;
            }
            
            case libtorrent::torrent_error_alert::alert_type: {
                auto* error_alert = libtorrent::alert_cast<libtorrent::torrent_error_alert>(alert);
                if (error_alert && s_error_callback) {
                    std::string name = error_alert->handle.status().name;
                    s_error_callback(name, error_alert->error.message());
                }
                break;
            }
            
            case libtorrent::state_update_alert::alert_type: {
                auto* update_alert = libtorrent::alert_cast<libtorrent::state_update_alert>(alert);
                if (update_alert && s_progress_callback) {
                    for (const auto& status : update_alert->status) {
                        std::string name = status.name;
                        s_progress_callback(name, status.progress);
                    }
                }
                break;
            }
        }
    }
#endif
}

std::string TorrentManager::getStatusString(int state) {
#ifndef NO_LIBTORRENT
    switch (state) {
        case libtorrent::torrent_status::checking_files: return "Vérification";
        case libtorrent::torrent_status::downloading_metadata: return "Métadonnées";
        case libtorrent::torrent_status::downloading: return "Téléchargement";
        case libtorrent::torrent_status::finished: return "Terminé";
        case libtorrent::torrent_status::seeding: return "Partage";
        case libtorrent::torrent_status::allocating: return "Allocation";
        case libtorrent::torrent_status::checking_resume_data: return "Reprise";
        default: return "Inconnu";
    }
#else
    return "Non disponible";
#endif
}

void TorrentManager::createTorrentFile(const std::string& file_path, const std::string& output_path) {
#ifndef NO_LIBTORRENT
    libtorrent::file_storage fs;
    libtorrent::add_files(fs, file_path);
    
    libtorrent::create_torrent torrent(fs);
    torrent.set_creator("PS4 Store P2P v1.0");
    torrent.set_comment("Package PS4 partagé via PS4 Store P2P");
    
    // Ajout de trackers publics
    torrent.add_tracker("udp://tracker.openbittorrent.com:80/announce");
    torrent.add_tracker("udp://tracker.opentrackr.org:1337/announce");
    
    // Génération des hashes
    libtorrent::set_piece_hashes(torrent, file_path.substr(0, file_path.find_last_of('/')));
    
    // Sauvegarde du fichier torrent
    std::ofstream out(output_path, std::ios::binary);
    libtorrent::bencode(std::ostream_iterator<char>(out), torrent.generate());
#endif
}