/**
 * PS4 Store P2P - Implémentation du Gestionnaire de Packages PKG
 */

#include "pkg/pkg_manager.h"
#include "utils/utils.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <thread>
#include <chrono>

// Structures pour le format PKG PS4
#pragma pack(push, 1)
struct PkgHeader {
    uint32_t magic;           // 0x7F434E54
    uint32_t type;
    uint32_t pkg_size;
    uint32_t data_offset;
    uint32_t data_size;
    uint32_t title_id_offset;
    uint32_t title_id_size;
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t unknown3;
    uint32_t unknown4;
    uint32_t unknown5;
    uint32_t content_id_offset;
    uint32_t content_id_size;
    uint32_t unknown6;
    uint32_t unknown7;
};

struct SfoHeader {
    uint32_t magic;           // 0x00505346
    uint32_t version;
    uint32_t key_table_start;
    uint32_t data_table_start;
    uint32_t tables_entries;
};

struct SfoIndexTableEntry {
    uint16_t key_offset;
    uint16_t param_fmt;
    uint32_t param_len;
    uint32_t param_max_len;
    uint32_t data_offset;
};
#pragma pack(pop)

// Variables statiques
std::string PkgManager::s_install_path = "/user/app";
std::string PkgManager::s_temp_path = "/data/ps4_store/temp";
InstallProgress PkgManager::s_current_install;
bool PkgManager::s_install_in_progress = false;

InstallProgressCallback PkgManager::s_progress_callback = nullptr;
InstallCompleteCallback PkgManager::s_complete_callback = nullptr;

int PkgManager::initialize() {
    LOG_INFO("Initialisation du gestionnaire de packages...");
    
    // Création des dossiers nécessaires
    if (!Utils::directoryExists(s_install_path)) {
        if (!Utils::createDirectory(s_install_path)) {
            LOG_ERROR("Impossible de créer le dossier d'installation: " + s_install_path);
            return -1;
        }
    }
    
    if (!Utils::directoryExists(s_temp_path)) {
        if (!Utils::createDirectory(s_temp_path)) {
            LOG_ERROR("Impossible de créer le dossier temporaire: " + s_temp_path);
            return -1;
        }
    }
    
    // Initialisation de l'état d'installation
    s_current_install = {};
    s_install_in_progress = false;
    
    LOG_INFO("Gestionnaire de packages initialisé avec succès");
    return 0;
}

void PkgManager::cleanup() {
    LOG_INFO("Nettoyage du gestionnaire de packages...");
    
    // Annulation de l'installation en cours si nécessaire
    if (s_install_in_progress) {
        cancelCurrentInstall();
    }
    
    // Nettoyage des fichiers temporaires
    cleanupTempFiles();
    
    LOG_INFO("Gestionnaire de packages nettoyé");
}

void PkgManager::update() {
    // Mise à jour du progrès d'installation si nécessaire
    if (s_install_in_progress && s_progress_callback) {
        s_progress_callback(s_current_install);
    }
}

PackageInfo PkgManager::analyzePackage(const std::string& pkg_path) {
    PackageInfo info = {};
    info.file_path = pkg_path;
    info.is_valid = false;
    
    if (!Utils::fileExists(pkg_path)) {
        LOG_ERROR("Fichier PKG non trouvé: " + pkg_path);
        return info;
    }
    
    LOG_INFO("Analyse du package: " + pkg_path);
    
    try {
        // Obtention de la taille du fichier
        info.file_size = Utils::getFileSize(pkg_path);
        
        // Parsing de l'en-tête PKG
        if (!parsePkgHeader(pkg_path, info)) {
            LOG_ERROR("Erreur lors du parsing de l'en-tête PKG");
            return info;
        }
        
        // Extraction des métadonnées
        if (!extractPkgMetadata(pkg_path, info)) {
            LOG_ERROR("Erreur lors de l'extraction des métadonnées");
            return info;
        }
        
        // Validation de la structure
        if (!validatePkgStructure(pkg_path)) {
            LOG_ERROR("Structure PKG invalide");
            return info;
        }
        
        // Calcul du checksum
        info.checksum_sha256 = calculateSHA256(pkg_path);
        
        info.is_valid = true;
        LOG_INFO("Package analysé avec succès: " + info.title);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception lors de l'analyse du package: " + std::string(e.what()));
    }
    
    return info;
}

bool PkgManager::verifyPackage(const std::string& pkg_path, const std::string& expected_checksum) {
    LOG_INFO("Vérification du package: " + pkg_path);
    
    if (!Utils::fileExists(pkg_path)) {
        LOG_ERROR("Fichier PKG non trouvé");
        return false;
    }
    
    // Vérification de la structure de base
    if (!validatePkgStructure(pkg_path)) {
        LOG_ERROR("Structure PKG invalide");
        return false;
    }
    
    // Vérification du checksum si fourni
    if (!expected_checksum.empty()) {
        std::string actual_checksum = calculateSHA256(pkg_path);
        if (actual_checksum != expected_checksum) {
            LOG_ERROR("Checksum invalide - Attendu: " + expected_checksum + ", Obtenu: " + actual_checksum);
            return false;
        }
    }
    
    LOG_INFO("Package vérifié avec succès");
    return true;
}

bool PkgManager::installPackage(const std::string& pkg_path, bool force_install) {
    if (s_install_in_progress) {
        LOG_WARNING("Installation déjà en cours");
        return false;
    }
    
    LOG_INFO("Démarrage de l'installation: " + pkg_path);
    
    // Analyse du package
    PackageInfo info = analyzePackage(pkg_path);
    if (!info.is_valid) {
        LOG_ERROR("Package invalide, installation annulée");
        return false;
    }
    
    // Vérification si déjà installé
    if (!force_install && isPackageInstalled(info.title_id)) {
        LOG_WARNING("Package déjà installé: " + info.title_id);
        return false;
    }
    
    // Vérification de l'espace disque
    if (!checkDiskSpace(info.file_size * 2)) { // x2 pour l'extraction
        LOG_ERROR("Espace disque insuffisant");
        return false;
    }
    
    // Initialisation du suivi d'installation
    s_current_install = {};
    s_current_install.package_name = info.title;
    s_current_install.status = InstallStatus::COPYING;
    s_current_install.progress = 0.0f;
    s_current_install.total_bytes = info.file_size;
    s_current_install.bytes_copied = 0;
    s_install_in_progress = true;
    
    // Lancement de l'installation dans un thread séparé
    std::thread install_thread([pkg_path, info]() {
        bool success = false;
        
        try {
            // Étape 1: Copie vers le dossier temporaire
            updateInstallProgress("Copie du package...", 0.1f);
            std::string temp_pkg = s_temp_path + "/" + info.title_id + ".pkg";
            
            if (!copyFileWithProgress(pkg_path, temp_pkg)) {
                throw std::runtime_error("Erreur lors de la copie");
            }
            
            // Étape 2: Vérification
            updateInstallProgress("Vérification...", 0.3f);
            s_current_install.status = InstallStatus::VERIFYING;
            
            if (!verifyPackage(temp_pkg)) {
                throw std::runtime_error("Vérification échouée");
            }
            
            // Étape 3: Installation via Debug Settings
            updateInstallProgress("Installation...", 0.5f);
            s_current_install.status = InstallStatus::INSTALLING;
            
            if (!triggerDebugInstall(temp_pkg)) {
                throw std::runtime_error("Installation échouée");
            }
            
            // Étape 4: Finalisation
            updateInstallProgress("Finalisation...", 0.9f);
            
            // Nettoyage du fichier temporaire
            Utils::deleteFile(temp_pkg);
            
            updateInstallProgress("Terminé", 1.0f);
            s_current_install.status = InstallStatus::COMPLETED;
            success = true;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Erreur d'installation: " + std::string(e.what()));
            s_current_install.status = InstallStatus::FAILED;
            s_current_install.error_message = e.what();
        }
        
        s_install_in_progress = false;
        
        if (s_complete_callback) {
            s_complete_callback(s_current_install.package_name, success);
        }
    });
    
    install_thread.detach();
    return true;
}

bool PkgManager::uninstallPackage(const std::string& title_id) {
    LOG_INFO("Désinstallation du package: " + title_id);
    
    std::string app_path = s_install_path + "/" + title_id;
    
    if (!Utils::directoryExists(app_path)) {
        LOG_WARNING("Package non installé: " + title_id);
        return false;
    }
    
    try {
        // Suppression du dossier de l'application
        if (!Utils::deleteDirectory(app_path)) {
            LOG_ERROR("Erreur lors de la suppression du dossier");
            return false;
        }
        
        LOG_INFO("Package désinstallé avec succès: " + title_id);
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de la désinstallation: " + std::string(e.what()));
        return false;
    }
}

std::vector<PackageInfo> PkgManager::getInstalledPackages() {
    std::vector<PackageInfo> packages;
    
    if (!Utils::directoryExists(s_install_path)) {
        return packages;
    }
    
    try {
        std::vector<std::string> dirs = Utils::listFiles(s_install_path);
        
        for (const std::string& dir : dirs) {
            std::string full_path = s_install_path + "/" + dir;
            if (Utils::directoryExists(full_path)) {
                PackageInfo info = getInstalledPackageInfo(dir);
                if (!info.title_id.empty()) {
                    packages.push_back(info);
                }
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de la récupération des packages installés: " + std::string(e.what()));
    }
    
    return packages;
}

bool PkgManager::isPackageInstalled(const std::string& title_id) {
    std::string app_path = s_install_path + "/" + title_id;
    return Utils::directoryExists(app_path);
}

PackageInfo PkgManager::getInstalledPackageInfo(const std::string& title_id) {
    PackageInfo info = {};
    info.title_id = title_id;
    info.is_installed = true;
    
    std::string app_path = s_install_path + "/" + title_id;
    std::string sfo_path = app_path + "/sce_sys/param.sfo";
    
    if (!Utils::fileExists(sfo_path)) {
        return info;
    }
    
    try {
        // Lecture du fichier param.sfo pour obtenir les métadonnées
        std::ifstream file(sfo_path, std::ios::binary);
        if (!file.is_open()) {
            return info;
        }
        
        // Parsing basique du SFO (simplifié)
        SfoHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        
        if (header.magic == 0x00505346) { // "PSF\0"
            info.title = "Application installée";
            info.version = "1.00";
            info.category = "gd";
        }
        
        file.close();
        
        // Obtention de la taille installée
        info.file_size = 0; // TODO: Calculer la taille du dossier
        info.install_path = app_path;
        
        // Recherche de l'icône
        std::string icon_path = app_path + "/sce_sys/icon0.png";
        if (Utils::fileExists(icon_path)) {
            info.icon_path = icon_path;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de la lecture des infos du package: " + std::string(e.what()));
    }
    
    return info;
}

InstallProgress PkgManager::getCurrentInstallProgress() {
    return s_current_install;
}

bool PkgManager::cancelCurrentInstall() {
    if (!s_install_in_progress) {
        return false;
    }
    
    LOG_INFO("Annulation de l'installation en cours");
    
    s_current_install.status = InstallStatus::CANCELLED;
    s_install_in_progress = false;
    
    // Nettoyage des fichiers temporaires
    cleanupTempFiles();
    
    return true;
}

bool PkgManager::copyPackageToInstallDir(const std::string& source_path, const std::string& dest_path) {
    return copyFileWithProgress(source_path, dest_path);
}

bool PkgManager::triggerDebugInstall(const std::string& pkg_path) {
    LOG_INFO("Déclenchement de l'installation via Debug Settings");
    
    // Sur PS4, ceci nécessiterait l'utilisation des APIs système
    // Pour le moment, on simule l'installation
    
    try {
        // Simulation d'une installation
        for (int i = 0; i <= 100; i += 10) {
            updateInstallProgress("Installation en cours...", 0.5f + (i / 100.0f) * 0.4f);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        LOG_INFO("Installation simulée terminée");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors de l'installation: " + std::string(e.what()));
        return false;
    }
}

void PkgManager::cleanupTempFiles() {
    if (!Utils::directoryExists(s_temp_path)) {
        return;
    }
    
    try {
        std::vector<std::string> files = Utils::listFiles(s_temp_path, ".pkg");
        for (const std::string& file : files) {
            std::string full_path = s_temp_path + "/" + file;
            Utils::deleteFile(full_path);
        }
        
        LOG_DEBUG("Fichiers temporaires nettoyés");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Erreur lors du nettoyage: " + std::string(e.what()));
    }
}

void PkgManager::setInstallProgressCallback(InstallProgressCallback callback) {
    s_progress_callback = callback;
}

void PkgManager::setInstallCompleteCallback(InstallCompleteCallback callback) {
    s_complete_callback = callback;
}

std::string PkgManager::calculateSHA256(const std::string& file_path) {
    // Implémentation simplifiée - dans un vrai projet, utiliser une lib crypto
    LOG_DEBUG("Calcul du SHA256 pour: " + file_path);
    
    // Pour le moment, retourner un hash fictif
    return "sha256_placeholder_" + std::to_string(Utils::getFileSize(file_path));
}

int64_t PkgManager::getAvailableDiskSpace(const std::string& path) {
    // Implémentation simplifiée - sur PS4, utiliser les APIs système
    return 50LL * 1024 * 1024 * 1024; // 50 GB fictifs
}

bool PkgManager::checkDiskSpace(int64_t required_space) {
    int64_t available = getAvailableDiskSpace(s_install_path);
    return available >= required_space;
}

std::string PkgManager::getDefaultInstallPath() {
    return s_install_path;
}

void PkgManager::setInstallPath(const std::string& path) {
    s_install_path = path;
    LOG_INFO("Chemin d'installation défini: " + path);
}

// Méthodes privées
bool PkgManager::parsePkgHeader(const std::string& pkg_path, PackageInfo& info) {
    std::ifstream file(pkg_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    PkgHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (header.magic != 0x7F434E54) { // Magic PKG
        file.close();
        return false;
    }
    
    // Lecture du Title ID si disponible
    if (header.title_id_offset > 0 && header.title_id_size > 0) {
        file.seekg(header.title_id_offset);
        std::vector<char> title_id_data(header.title_id_size);
        file.read(title_id_data.data(), header.title_id_size);
        info.title_id = std::string(title_id_data.data(), header.title_id_size);
    }
    
    // Lecture du Content ID si disponible
    if (header.content_id_offset > 0 && header.content_id_size > 0) {
        file.seekg(header.content_id_offset);
        std::vector<char> content_id_data(header.content_id_size);
        file.read(content_id_data.data(), header.content_id_size);
        info.content_id = std::string(content_id_data.data(), header.content_id_size);
    }
    
    file.close();
    return true;
}

bool PkgManager::extractPkgMetadata(const std::string& pkg_path, PackageInfo& info) {
    // Extraction simplifiée des métadonnées
    // Dans un vrai projet, il faudrait parser complètement le format PKG
    
    info.title = "Package PS4";
    info.version = "1.00";
    info.category = "gd";
    info.publisher = "Inconnu";
    info.description = "Package installé via PS4 Store P2P";
    
    return true;
}

bool PkgManager::validatePkgStructure(const std::string& pkg_path) {
    std::ifstream file(pkg_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Vérification du magic number
    uint32_t magic;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    
    file.close();
    return magic == 0x7F434E54;
}

void PkgManager::updateInstallProgress(const std::string& operation, float progress) {
    s_current_install.current_operation = operation;
    s_current_install.progress = progress;
    
    LOG_DEBUG("Progrès d'installation: " + operation + " (" + 
              Utils::formatPercentage(progress) + ")");
}

bool PkgManager::copyFileWithProgress(const std::string& source, const std::string& dest) {
    std::ifstream src(source, std::ios::binary);
    std::ofstream dst(dest, std::ios::binary);
    
    if (!src.is_open() || !dst.is_open()) {
        return false;
    }
    
    const size_t buffer_size = 64 * 1024; // 64KB buffer
    std::vector<char> buffer(buffer_size);
    
    int64_t total_size = Utils::getFileSize(source);
    int64_t copied = 0;
    
    while (src.read(buffer.data(), buffer_size) || src.gcount() > 0) {
        dst.write(buffer.data(), src.gcount());
        copied += src.gcount();
        
        s_current_install.bytes_copied = copied;
        
        if (total_size > 0) {
            float progress = static_cast<float>(copied) / total_size;
            updateInstallProgress("Copie en cours...", progress * 0.2f); // 20% pour la copie
        }
    }
    
    src.close();
    dst.close();
    
    return copied == total_size;
}

std::string PkgManager::formatFileSize(int64_t bytes) {
    return Utils::formatFileSize(bytes);
}

bool PkgManager::createDirectoryRecursive(const std::string& path) {
    return Utils::createDirectory(path);
}