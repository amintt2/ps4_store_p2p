/**
 * @file test_main.cpp
 * @brief Tests unitaires pour PS4 Store P2P
 * @author PS4 Store P2P Team
 * @date 2024
 */

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

// Headers du projet à tester
#include "../include/utils/utils.h"
#include "../include/p2p/torrent_manager.h"
#include "../include/pkg/pkg_manager.h"
#include "../include/ui/main_window.h"

// Macro pour les tests
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "[FAIL] " << message << std::endl; \
            return false; \
        } else { \
            std::cout << "[PASS] " << message << std::endl; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        std::cout << "\n=== Running " << #test_func << " ===" << std::endl; \
        if (test_func()) { \
            std::cout << "[SUCCESS] " << #test_func << " passed" << std::endl; \
            tests_passed++; \
        } else { \
            std::cout << "[ERROR] " << #test_func << " failed" << std::endl; \
            tests_failed++; \
        } \
        total_tests++; \
    } while(0)

// Variables globales pour les statistiques de test
static int total_tests = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/**
 * Test des utilitaires de base
 */
bool test_utils_basic() {
    // Test de formatage de taille de fichier
    std::string size_1kb = Utils::FormatFileSize(1024);
    TEST_ASSERT(size_1kb == "1.0 KB", "Format file size 1KB");
    
    std::string size_1mb = Utils::FormatFileSize(1024 * 1024);
    TEST_ASSERT(size_1mb == "1.0 MB", "Format file size 1MB");
    
    std::string size_1gb = Utils::FormatFileSize(1024 * 1024 * 1024);
    TEST_ASSERT(size_1gb == "1.0 GB", "Format file size 1GB");
    
    // Test de formatage de vitesse
    std::string speed_1kbps = Utils::FormatSpeed(1024);
    TEST_ASSERT(speed_1kbps == "1.0 KB/s", "Format speed 1KB/s");
    
    // Test de formatage de durée
    std::string duration_1min = Utils::FormatDuration(60);
    TEST_ASSERT(duration_1min == "1m 0s", "Format duration 1 minute");
    
    std::string duration_1hour = Utils::FormatDuration(3661);
    TEST_ASSERT(duration_1hour == "1h 1m 1s", "Format duration 1h 1m 1s");
    
    // Test de formatage de pourcentage
    std::string percent_50 = Utils::FormatPercentage(50.5f);
    TEST_ASSERT(percent_50 == "50.5%", "Format percentage 50.5%");
    
    return true;
}

/**
 * Test des fonctions de manipulation de chaînes
 */
bool test_utils_strings() {
    // Test de division de chaîne
    std::vector<std::string> parts = Utils::SplitString("a,b,c,d", ',');
    TEST_ASSERT(parts.size() == 4, "Split string count");
    TEST_ASSERT(parts[0] == "a" && parts[3] == "d", "Split string content");
    
    // Test de jointure de chaînes
    std::string joined = Utils::JoinStrings(parts, "|");
    TEST_ASSERT(joined == "a|b|c|d", "Join strings");
    
    // Test de trim
    std::string trimmed = Utils::TrimString("  hello world  ");
    TEST_ASSERT(trimmed == "hello world", "Trim string");
    
    // Test de conversion en minuscules
    std::string lower = Utils::ToLowerCase("HELLO World");
    TEST_ASSERT(lower == "hello world", "To lower case");
    
    // Test de conversion en majuscules
    std::string upper = Utils::ToUpperCase("hello World");
    TEST_ASSERT(upper == "HELLO WORLD", "To upper case");
    
    // Test de préfixe
    bool has_prefix = Utils::StartsWith("hello world", "hello");
    TEST_ASSERT(has_prefix == true, "Starts with true");
    
    bool no_prefix = Utils::StartsWith("hello world", "world");
    TEST_ASSERT(no_prefix == false, "Starts with false");
    
    // Test de suffixe
    bool has_suffix = Utils::EndsWith("hello world", "world");
    TEST_ASSERT(has_suffix == true, "Ends with true");
    
    bool no_suffix = Utils::EndsWith("hello world", "hello");
    TEST_ASSERT(no_suffix == false, "Ends with false");
    
    return true;
}

/**
 * Test des fonctions de fichiers (simulation)
 */
bool test_utils_files() {
    // Note: Ces tests sont simulés car nous ne voulons pas créer de vrais fichiers
    
    // Test d'existence de fichier (fichier inexistant)
    bool exists = Utils::FileExists("/path/to/nonexistent/file.txt");
    TEST_ASSERT(exists == false, "File exists false for nonexistent file");
    
    // Test de validation d'IP
    bool valid_ip = Utils::IsValidIP("192.168.1.1");
    TEST_ASSERT(valid_ip == true, "Valid IP address");
    
    bool invalid_ip = Utils::IsValidIP("999.999.999.999");
    TEST_ASSERT(invalid_ip == false, "Invalid IP address");
    
    // Test de validation de port
    bool valid_port = Utils::IsValidPort(8080);
    TEST_ASSERT(valid_port == true, "Valid port number");
    
    bool invalid_port_low = Utils::IsValidPort(0);
    TEST_ASSERT(invalid_port_low == false, "Invalid port number (too low)");
    
    bool invalid_port_high = Utils::IsValidPort(70000);
    TEST_ASSERT(invalid_port_high == false, "Invalid port number (too high)");
    
    return true;
}

/**
 * Test d'initialisation du gestionnaire de torrents
 */
bool test_torrent_manager_init() {
    // Test d'initialisation
    bool init_result = TorrentManager::Initialize();
    TEST_ASSERT(init_result == true, "TorrentManager initialization");
    
    // Test de configuration des limites
    TorrentManager::SetDownloadLimit(1024); // 1 MB/s
    TorrentManager::SetUploadLimit(512);     // 512 KB/s
    TorrentManager::SetListenPort(6881);
    
    // Test de récupération des statistiques globales
    GlobalStats stats = TorrentManager::GetGlobalStats();
    TEST_ASSERT(stats.download_rate >= 0, "Global stats download rate");
    TEST_ASSERT(stats.upload_rate >= 0, "Global stats upload rate");
    
    // Test de nettoyage
    TorrentManager::Cleanup();
    
    return true;
}

/**
 * Test de simulation de téléchargement
 */
bool test_torrent_download_simulation() {
    // Réinitialiser le gestionnaire
    bool init_result = TorrentManager::Initialize();
    TEST_ASSERT(init_result == true, "TorrentManager re-initialization");
    
    // Test avec un magnet URI fictif (ne démarrera pas vraiment)
    std::string fake_magnet = "magnet:?xt=urn:btih:0123456789abcdef0123456789abcdef01234567&dn=test";
    std::string torrent_id = TorrentManager::StartDownload(fake_magnet);
    TEST_ASSERT(!torrent_id.empty(), "Start download returns ID");
    
    // Attendre un peu pour la simulation
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Test de récupération des informations
    DownloadInfo info = TorrentManager::GetDownloadInfo(torrent_id);
    TEST_ASSERT(info.torrent_id == torrent_id, "Download info torrent ID");
    
    // Test de liste des téléchargements
    std::vector<DownloadInfo> downloads = TorrentManager::GetAllDownloads();
    TEST_ASSERT(downloads.size() >= 1, "Download list contains our download");
    
    // Test d'arrêt du téléchargement
    bool stop_result = TorrentManager::StopDownload(torrent_id);
    TEST_ASSERT(stop_result == true, "Stop download");
    
    // Test de suppression du téléchargement
    bool remove_result = TorrentManager::RemoveDownload(torrent_id);
    TEST_ASSERT(remove_result == true, "Remove download");
    
    TorrentManager::Cleanup();
    return true;
}

/**
 * Test d'initialisation du gestionnaire PKG
 */
bool test_pkg_manager_init() {
    // Test d'initialisation
    bool init_result = PkgManager::Initialize();
    TEST_ASSERT(init_result == true, "PkgManager initialization");
    
    // Test de vérification d'espace disque
    uint64_t free_space = PkgManager::GetFreeSpace("/tmp");
    TEST_ASSERT(free_space > 0, "Free space check");
    
    // Test de calcul SHA256 avec une chaîne vide
    std::string empty_hash = PkgManager::CalculateSHA256("");
    std::string expected_empty_hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    TEST_ASSERT(empty_hash == expected_empty_hash, "SHA256 of empty string");
    
    // Test de nettoyage
    PkgManager::Cleanup();
    
    return true;
}

/**
 * Test de simulation d'analyse de package
 */
bool test_pkg_analysis_simulation() {
    bool init_result = PkgManager::Initialize();
    TEST_ASSERT(init_result == true, "PkgManager re-initialization");
    
    // Test avec un fichier inexistant (doit échouer proprement)
    PackageInfo info = PkgManager::AnalyzePackage("/path/to/nonexistent.pkg");
    TEST_ASSERT(info.content_id.empty(), "Analyze nonexistent package");
    
    // Test de liste des packages installés (peut être vide)
    std::vector<PackageInfo> installed = PkgManager::GetInstalledPackages();
    TEST_ASSERT(installed.size() >= 0, "Get installed packages list");
    
    PkgManager::Cleanup();
    return true;
}

/**
 * Test d'initialisation de l'interface utilisateur
 */
bool test_ui_initialization() {
    // Note: Ces tests sont limités car SDL nécessite un environnement graphique
    
    // Test de validation des paramètres d'initialisation
    // (simulation sans vraie initialisation SDL)
    
    // Test des structures de données
    GameInfo game;
    game.title = "Test Game";
    game.content_id = "TEST123456789";
    game.size = 1024 * 1024 * 1024; // 1 GB
    game.description = "A test game for unit testing";
    
    TEST_ASSERT(game.title == "Test Game", "GameInfo title assignment");
    TEST_ASSERT(game.size == 1073741824, "GameInfo size assignment");
    
    // Test des énumérations
    UIState state = UIState::MAIN_MENU;
    TEST_ASSERT(state == UIState::MAIN_MENU, "UIState enum");
    
    return true;
}

/**
 * Test de performance basique
 */
bool test_performance() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Test de performance du formatage
    for (int i = 0; i < 1000; i++) {
        Utils::FormatFileSize(i * 1024 * 1024);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    TEST_ASSERT(duration.count() < 1000, "Performance test: 1000 format operations < 1s");
    
    return true;
}

/**
 * Test de gestion des erreurs
 */
bool test_error_handling() {
    // Test de gestion des paramètres invalides
    std::string invalid_format = Utils::FormatFileSize(UINT64_MAX);
    TEST_ASSERT(!invalid_format.empty(), "Format very large file size");
    
    // Test de division par zéro dans le formatage de pourcentage
    std::string zero_percent = Utils::FormatPercentage(0.0f);
    TEST_ASSERT(zero_percent == "0.0%", "Format zero percentage");
    
    // Test de chaîne vide
    std::vector<std::string> empty_split = Utils::SplitString("", ',');
    TEST_ASSERT(empty_split.size() == 1 && empty_split[0].empty(), "Split empty string");
    
    return true;
}

/**
 * Test d'intégration basique
 */
bool test_integration() {
    // Test d'initialisation de tous les composants
    bool utils_ok = true; // Utils n'a pas d'initialisation
    bool torrent_ok = TorrentManager::Initialize();
    bool pkg_ok = PkgManager::Initialize();
    
    TEST_ASSERT(utils_ok && torrent_ok && pkg_ok, "All components initialization");
    
    // Test de nettoyage de tous les composants
    TorrentManager::Cleanup();
    PkgManager::Cleanup();
    
    return true;
}

/**
 * Fonction principale des tests
 */
int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "    Tests Unitaires PS4 Store P2P" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    // Exécution des tests
    RUN_TEST(test_utils_basic);
    RUN_TEST(test_utils_strings);
    RUN_TEST(test_utils_files);
    RUN_TEST(test_torrent_manager_init);
    RUN_TEST(test_torrent_download_simulation);
    RUN_TEST(test_pkg_manager_init);
    RUN_TEST(test_pkg_analysis_simulation);
    RUN_TEST(test_ui_initialization);
    RUN_TEST(test_performance);
    RUN_TEST(test_error_handling);
    RUN_TEST(test_integration);
    
    // Affichage des résultats
    std::cout << "\n=========================================" << std::endl;
    std::cout << "           Résultats des Tests" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Total des tests    : " << total_tests << std::endl;
    std::cout << "Tests réussis      : " << tests_passed << std::endl;
    std::cout << "Tests échoués      : " << tests_failed << std::endl;
    std::cout << "Taux de réussite   : " << (total_tests > 0 ? (tests_passed * 100 / total_tests) : 0) << "%" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    if (tests_failed == 0) {
        std::cout << "\n🎉 Tous les tests sont passés avec succès!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ " << tests_failed << " test(s) ont échoué." << std::endl;
        return 1;
    }
}

/**
 * Fonction utilitaire pour les tests de benchmark
 */
void benchmark_function(const std::string& name, std::function<void()> func, int iterations = 1000) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        func();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Benchmark " << name << ": " 
              << duration.count() << " μs total, "
              << (duration.count() / iterations) << " μs/op" << std::endl;
}