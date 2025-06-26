/**
 * PS4 Store P2P - Utilitaires
 * 
 * Fonctions utilitaires générales pour le projet
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <map>
#include <chrono>

// Macros pour le logging
#define LOG_INFO(msg) Utils::log(Utils::LogLevel::INFO, msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Utils::log(Utils::LogLevel::WARNING, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Utils::log(Utils::LogLevel::ERROR, msg, __FILE__, __LINE__)
#define LOG_DEBUG(msg) Utils::log(Utils::LogLevel::DEBUG, msg, __FILE__, __LINE__)

class Utils {
public:
    // Niveaux de log
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };
    
    /**
     * Initialise les utilitaires
     * @return 0 en cas de succès
     */
    static int initialize();
    
    /**
     * Nettoie les ressources
     */
    static void cleanup();
    
    // === GESTION DES FICHIERS ===
    
    /**
     * Vérifie si un fichier existe
     * @param path Chemin du fichier
     * @return true si le fichier existe
     */
    static bool fileExists(const std::string& path);
    
    /**
     * Vérifie si un dossier existe
     * @param path Chemin du dossier
     * @return true si le dossier existe
     */
    static bool directoryExists(const std::string& path);
    
    /**
     * Crée un dossier récursivement
     * @param path Chemin du dossier
     * @return true en cas de succès
     */
    static bool createDirectory(const std::string& path);
    
    /**
     * Supprime un fichier
     * @param path Chemin du fichier
     * @return true en cas de succès
     */
    static bool deleteFile(const std::string& path);
    
    /**
     * Supprime un dossier récursivement
     * @param path Chemin du dossier
     * @return true en cas de succès
     */
    static bool deleteDirectory(const std::string& path);
    
    /**
     * Obtient la taille d'un fichier
     * @param path Chemin du fichier
     * @return Taille en bytes, -1 en cas d'erreur
     */
    static int64_t getFileSize(const std::string& path);
    
    /**
     * Copie un fichier
     * @param source Fichier source
     * @param destination Fichier destination
     * @return true en cas de succès
     */
    static bool copyFile(const std::string& source, const std::string& destination);
    
    /**
     * Déplace un fichier
     * @param source Fichier source
     * @param destination Fichier destination
     * @return true en cas de succès
     */
    static bool moveFile(const std::string& source, const std::string& destination);
    
    /**
     * Liste les fichiers d'un dossier
     * @param directory_path Chemin du dossier
     * @param extension Filtre par extension (optionnel)
     * @return Liste des fichiers
     */
    static std::vector<std::string> listFiles(const std::string& directory_path, 
                                             const std::string& extension = "");
    
    // === GESTION DES CHAÎNES ===
    
    /**
     * Divise une chaîne selon un délimiteur
     * @param str Chaîne à diviser
     * @param delimiter Délimiteur
     * @return Vecteur de sous-chaînes
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);
    
    /**
     * Joint des chaînes avec un délimiteur
     * @param strings Chaînes à joindre
     * @param delimiter Délimiteur
     * @return Chaîne jointe
     */
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter);
    
    /**
     * Supprime les espaces en début et fin de chaîne
     * @param str Chaîne à traiter
     * @return Chaîne sans espaces
     */
    static std::string trim(const std::string& str);
    
    /**
     * Convertit en minuscules
     * @param str Chaîne à convertir
     * @return Chaîne en minuscules
     */
    static std::string toLowerCase(const std::string& str);
    
    /**
     * Convertit en majuscules
     * @param str Chaîne à convertir
     * @return Chaîne en majuscules
     */
    static std::string toUpperCase(const std::string& str);
    
    /**
     * Vérifie si une chaîne commence par un préfixe
     * @param str Chaîne à vérifier
     * @param prefix Préfixe
     * @return true si la chaîne commence par le préfixe
     */
    static bool startsWith(const std::string& str, const std::string& prefix);
    
    /**
     * Vérifie si une chaîne se termine par un suffixe
     * @param str Chaîne à vérifier
     * @param suffix Suffixe
     * @return true si la chaîne se termine par le suffixe
     */
    static bool endsWith(const std::string& str, const std::string& suffix);
    
    // === FORMATAGE ===
    
    /**
     * Formate une taille de fichier en format lisible
     * @param bytes Taille en bytes
     * @return Chaîne formatée (ex: "1.5 MB")
     */
    static std::string formatFileSize(int64_t bytes);
    
    /**
     * Formate une vitesse en format lisible
     * @param bytes_per_second Vitesse en bytes/sec
     * @return Chaîne formatée (ex: "1.2 MB/s")
     */
    static std::string formatSpeed(int bytes_per_second);
    
    /**
     * Formate une durée en format lisible
     * @param seconds Durée en secondes
     * @return Chaîne formatée (ex: "1h 23m 45s")
     */
    static std::string formatDuration(int seconds);
    
    /**
     * Formate un pourcentage
     * @param value Valeur entre 0.0 et 1.0
     * @param decimals Nombre de décimales
     * @return Chaîne formatée (ex: "75.5%")
     */
    static std::string formatPercentage(float value, int decimals = 1);
    
    // === TEMPS ===
    
    /**
     * Obtient le timestamp actuel en millisecondes
     * @return Timestamp
     */
    static int64_t getCurrentTimestamp();
    
    /**
     * Obtient la date/heure actuelle formatée
     * @param format Format de la date (par défaut: "%Y-%m-%d %H:%M:%S")
     * @return Chaîne formatée
     */
    static std::string getCurrentDateTime(const std::string& format = "%Y-%m-%d %H:%M:%S");
    
    /**
     * Mesure le temps d'exécution d'une fonction
     * @param func Fonction à mesurer
     * @return Durée en millisecondes
     */
    template<typename Func>
    static int64_t measureExecutionTime(Func func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }
    
    // === LOGGING ===
    
    /**
     * Écrit un message de log
     * @param level Niveau de log
     * @param message Message
     * @param file Fichier source
     * @param line Ligne source
     */
    static void log(LogLevel level, const std::string& message, 
                   const char* file = nullptr, int line = 0);
    
    /**
     * Définit le niveau de log minimum
     * @param level Niveau minimum
     */
    static void setLogLevel(LogLevel level);
    
    /**
     * Active/désactive l'écriture des logs dans un fichier
     * @param enabled Activer l'écriture
     * @param log_file Chemin du fichier de log
     */
    static void setFileLogging(bool enabled, const std::string& log_file = "ps4_store.log");
    
    // === CONFIGURATION ===
    
    /**
     * Charge un fichier de configuration JSON
     * @param config_file Chemin du fichier
     * @return Map des paramètres
     */
    static std::map<std::string, std::string> loadConfig(const std::string& config_file);
    
    /**
     * Sauvegarde un fichier de configuration JSON
     * @param config_file Chemin du fichier
     * @param config Map des paramètres
     * @return true en cas de succès
     */
    static bool saveConfig(const std::string& config_file, 
                          const std::map<std::string, std::string>& config);
    
    // === RÉSEAU ===
    
    /**
     * Vérifie si une adresse IP est valide
     * @param ip Adresse IP
     * @return true si valide
     */
    static bool isValidIP(const std::string& ip);
    
    /**
     * Vérifie si un port est valide
     * @param port Port à vérifier
     * @return true si valide (1-65535)
     */
    static bool isValidPort(int port);
    
    /**
     * Obtient l'adresse IP locale
     * @return Adresse IP locale
     */
    static std::string getLocalIP();
    
private:
    static LogLevel s_log_level;
    static bool s_file_logging_enabled;
    static std::string s_log_file;
    
    static std::string getLogLevelString(LogLevel level);
    static void writeToLogFile(const std::string& message);
};

#endif // UTILS_H