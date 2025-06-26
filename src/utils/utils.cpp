/**
 * PS4 Store P2P - Implémentation des Utilitaires
 */

#include "utils/utils.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <sys/stat.h>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#endif

// Variables statiques
Utils::LogLevel Utils::s_log_level = Utils::LogLevel::INFO;
bool Utils::s_file_logging_enabled = false;
std::string Utils::s_log_file = "ps4_store.log";

int Utils::initialize() {
    // Initialisation du système de logging
    log(LogLevel::INFO, "Initialisation des utilitaires");
    
#ifdef _WIN32
    // Initialisation de Winsock pour les fonctions réseau
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        log(LogLevel::ERROR, "Erreur d'initialisation Winsock");
        return -1;
    }
#endif
    
    return 0;
}

void Utils::cleanup() {
    log(LogLevel::INFO, "Nettoyage des utilitaires");
    
#ifdef _WIN32
    WSACleanup();
#endif
}

// === GESTION DES FICHIERS ===

bool Utils::fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

bool Utils::directoryExists(const std::string& path) {
    try {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool Utils::createDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception& e) {
        log(LogLevel::ERROR, "Erreur lors de la création du dossier " + path + ": " + e.what());
        return false;
    }
}

bool Utils::deleteFile(const std::string& path) {
    try {
        return std::filesystem::remove(path);
    } catch (const std::exception& e) {
        log(LogLevel::ERROR, "Erreur lors de la suppression du fichier " + path + ": " + e.what());
        return false;
    }
}

bool Utils::deleteDirectory(const std::string& path) {
    try {
        return std::filesystem::remove_all(path) > 0;
    } catch (const std::exception& e) {
        log(LogLevel::ERROR, "Erreur lors de la suppression du dossier " + path + ": " + e.what());
        return false;
    }
}

int64_t Utils::getFileSize(const std::string& path) {
    try {
        return std::filesystem::file_size(path);
    } catch (const std::exception&) {
        return -1;
    }
}

bool Utils::copyFile(const std::string& source, const std::string& destination) {
    try {
        std::filesystem::copy_file(source, destination, 
                                  std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        log(LogLevel::ERROR, "Erreur lors de la copie de " + source + " vers " + destination + ": " + e.what());
        return false;
    }
}

bool Utils::moveFile(const std::string& source, const std::string& destination) {
    try {
        std::filesystem::rename(source, destination);
        return true;
    } catch (const std::exception& e) {
        log(LogLevel::ERROR, "Erreur lors du déplacement de " + source + " vers " + destination + ": " + e.what());
        return false;
    }
}

std::vector<std::string> Utils::listFiles(const std::string& directory_path, const std::string& extension) {
    std::vector<std::string> files;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                
                if (extension.empty() || endsWith(filename, extension)) {
                    files.push_back(filename);
                }
            } else if (entry.is_directory() && extension.empty()) {
                files.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::exception& e) {
        log(LogLevel::ERROR, "Erreur lors de la liste des fichiers dans " + directory_path + ": " + e.what());
    }
    
    return files;
}

// === GESTION DES CHAÎNES ===

std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string Utils::join(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << strings[0];
    
    for (size_t i = 1; i < strings.size(); ++i) {
        oss << delimiter << strings[i];
    }
    
    return oss.str();
}

std::string Utils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::string Utils::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string Utils::toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool Utils::startsWith(const std::string& str, const std::string& prefix) {
    return str.length() >= prefix.length() && 
           str.compare(0, prefix.length(), prefix) == 0;
}

bool Utils::endsWith(const std::string& str, const std::string& suffix) {
    return str.length() >= suffix.length() && 
           str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

// === FORMATAGE ===

std::string Utils::formatFileSize(int64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    const int num_units = sizeof(units) / sizeof(units[0]);
    
    double size = static_cast<double>(bytes);
    int unit_index = 0;
    
    while (size >= 1024.0 && unit_index < num_units - 1) {
        size /= 1024.0;
        unit_index++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[unit_index];
    return oss.str();
}

std::string Utils::formatSpeed(int bytes_per_second) {
    return formatFileSize(bytes_per_second) + "/s";
}

std::string Utils::formatDuration(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    std::ostringstream oss;
    
    if (hours > 0) {
        oss << hours << "h ";
    }
    if (minutes > 0 || hours > 0) {
        oss << minutes << "m ";
    }
    oss << secs << "s";
    
    return oss.str();
}

std::string Utils::formatPercentage(float value, int decimals) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(decimals) << (value * 100.0f) << "%";
    return oss.str();
}

// === TEMPS ===

int64_t Utils::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

std::string Utils::getCurrentDateTime(const std::string& format) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), format.c_str());
    return oss.str();
}

// === LOGGING ===

void Utils::log(LogLevel level, const std::string& message, const char* file, int line) {
    if (level < s_log_level) {
        return;
    }
    
    std::ostringstream oss;
    oss << "[" << getCurrentDateTime() << "] ";
    oss << "[" << getLogLevelString(level) << "] ";
    
    if (file && line > 0) {
        std::string filename = file;
        size_t pos = filename.find_last_of("/\\");
        if (pos != std::string::npos) {
            filename = filename.substr(pos + 1);
        }
        oss << "[" << filename << ":" << line << "] ";
    }
    
    oss << message;
    
    std::string log_message = oss.str();
    
    // Affichage sur la console
    std::cout << log_message << std::endl;
    
    // Écriture dans le fichier si activée
    if (s_file_logging_enabled) {
        writeToLogFile(log_message);
    }
}

void Utils::setLogLevel(LogLevel level) {
    s_log_level = level;
    log(LogLevel::INFO, "Niveau de log défini: " + getLogLevelString(level));
}

void Utils::setFileLogging(bool enabled, const std::string& log_file) {
    s_file_logging_enabled = enabled;
    s_log_file = log_file;
    
    if (enabled) {
        log(LogLevel::INFO, "Logging fichier activé: " + log_file);
    } else {
        log(LogLevel::INFO, "Logging fichier désactivé");
    }
}

// === CONFIGURATION ===

std::map<std::string, std::string> Utils::loadConfig(const std::string& config_file) {
    std::map<std::string, std::string> config;
    
    if (!fileExists(config_file)) {
        log(LogLevel::WARNING, "Fichier de configuration non trouvé: " + config_file);
        return config;
    }
    
    try {
        std::ifstream file(config_file);
        std::string line;
        
        while (std::getline(file, line)) {
            line = trim(line);
            
            // Ignorer les commentaires et lignes vides
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = trim(line.substr(0, pos));
                std::string value = trim(line.substr(pos + 1));
                config[key] = value;
            }
        }
        
        file.close();
        log(LogLevel::INFO, "Configuration chargée: " + std::to_string(config.size()) + " paramètres");
        
    } catch (const std::exception& e) {
        log(LogLevel::ERROR, "Erreur lors du chargement de la configuration: " + std::string(e.what()));
    }
    
    return config;
}

bool Utils::saveConfig(const std::string& config_file, const std::map<std::string, std::string>& config) {
    try {
        std::ofstream file(config_file);
        
        file << "# Configuration PS4 Store P2P" << std::endl;
        file << "# Généré le " << getCurrentDateTime() << std::endl;
        file << std::endl;
        
        for (const auto& pair : config) {
            file << pair.first << "=" << pair.second << std::endl;
        }
        
        file.close();
        log(LogLevel::INFO, "Configuration sauvegardée: " + std::to_string(config.size()) + " paramètres");
        return true;
        
    } catch (const std::exception& e) {
        log(LogLevel::ERROR, "Erreur lors de la sauvegarde de la configuration: " + std::string(e.what()));
        return false;
    }
}

// === RÉSEAU ===

bool Utils::isValidIP(const std::string& ip) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
    return result != 0;
}

bool Utils::isValidPort(int port) {
    return port > 0 && port <= 65535;
}

std::string Utils::getLocalIP() {
#ifdef _WIN32
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        return "127.0.0.1";
    }
    
    struct addrinfo hints, *info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(hostname, NULL, &hints, &info) != 0) {
        return "127.0.0.1";
    }
    
    struct sockaddr_in* addr_in = (struct sockaddr_in*)info->ai_addr;
    std::string ip = inet_ntoa(addr_in->sin_addr);
    
    freeaddrinfo(info);
    return ip;
#else
    struct ifaddrs *ifaddrs_ptr, *ifa;
    
    if (getifaddrs(&ifaddrs_ptr) == -1) {
        return "127.0.0.1";
    }
    
    for (ifa = ifaddrs_ptr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* addr_in = (struct sockaddr_in*)ifa->ifa_addr;
            std::string ip = inet_ntoa(addr_in->sin_addr);
            
            // Éviter l'adresse de loopback
            if (ip != "127.0.0.1") {
                freeifaddrs(ifaddrs_ptr);
                return ip;
            }
        }
    }
    
    freeifaddrs(ifaddrs_ptr);
    return "127.0.0.1";
#endif
}

// Méthodes privées
std::string Utils::getLogLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

void Utils::writeToLogFile(const std::string& message) {
    try {
        std::ofstream file(s_log_file, std::ios::app);
        if (file.is_open()) {
            file << message << std::endl;
            file.close();
        }
    } catch (const std::exception&) {
        // Ignorer les erreurs d'écriture de log pour éviter les boucles infinies
    }
}