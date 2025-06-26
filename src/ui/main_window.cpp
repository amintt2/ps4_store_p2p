/**
 * PS4 Store P2P - Implémentation de l'Interface Utilisateur Principale
 */

#include "ui/main_window.h"
#include "utils/utils.h"
#include "p2p/torrent_manager.h"
#include "pkg/pkg_manager.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <algorithm>

// Variables statiques
SDL_Window* MainWindow::s_window = nullptr;
SDL_Renderer* MainWindow::s_renderer = nullptr;
TTF_Font* MainWindow::s_font_large = nullptr;
TTF_Font* MainWindow::s_font_medium = nullptr;
TTF_Font* MainWindow::s_font_small = nullptr;

MainWindow::UIState MainWindow::s_current_state = MainWindow::UIState::MAIN_MENU;
std::vector<GameInfo> MainWindow::s_available_games;
std::vector<DownloadInfo> MainWindow::s_active_downloads;
int MainWindow::s_selected_index = 0;
int MainWindow::s_scroll_offset = 0;

std::string MainWindow::s_notification_text = "";
int64_t MainWindow::s_notification_time = 0;
const int64_t MainWindow::NOTIFICATION_DURATION = 3000; // 3 secondes

// Couleurs
const SDL_Color MainWindow::COLOR_BACKGROUND = {20, 25, 40, 255};
const SDL_Color MainWindow::COLOR_PRIMARY = {0, 122, 255, 255};
const SDL_Color MainWindow::COLOR_SECONDARY = {142, 142, 147, 255};
const SDL_Color MainWindow::COLOR_TEXT = {255, 255, 255, 255};
const SDL_Color MainWindow::COLOR_TEXT_SECONDARY = {174, 174, 178, 255};
const SDL_Color MainWindow::COLOR_SUCCESS = {52, 199, 89, 255};
const SDL_Color MainWindow::COLOR_WARNING = {255, 149, 0, 255};
const SDL_Color MainWindow::COLOR_ERROR = {255, 59, 48, 255};

int MainWindow::initialize() {
    LOG_INFO("Initialisation de l'interface utilisateur...");
    
    // Initialisation de SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        LOG_ERROR("Erreur d'initialisation SDL: " + std::string(SDL_GetError()));
        return -1;
    }
    
    // Initialisation de SDL_ttf
    if (TTF_Init() == -1) {
        LOG_ERROR("Erreur d'initialisation SDL_ttf: " + std::string(TTF_GetError()));
        SDL_Quit();
        return -1;
    }
    
    // Création de la fenêtre (plein écran pour PS4)
    s_window = SDL_CreateWindow(
        "PS4 Store P2P",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1920, 1080,  // Résolution PS4
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    
    if (!s_window) {
        LOG_ERROR("Erreur de création de fenêtre: " + std::string(SDL_GetError()));
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    
    // Création du renderer
    s_renderer = SDL_CreateRenderer(s_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!s_renderer) {
        LOG_ERROR("Erreur de création du renderer: " + std::string(SDL_GetError()));
        SDL_DestroyWindow(s_window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    
    // Chargement des polices
    if (!loadFonts()) {
        LOG_ERROR("Erreur de chargement des polices");
        cleanup();
        return -1;
    }
    
    // Initialisation des données de test
    initializeTestData();
    
    // Configuration des callbacks
    TorrentManager::setProgressCallback([](const std::string& name, float progress) {
        updateDownloadProgress(name, progress);
    });
    
    TorrentManager::setCompleteCallback([](const std::string& name, const std::string& path) {
        showNotification("Téléchargement terminé: " + name, NotificationType::SUCCESS);
        updateGameList();
    });
    
    TorrentManager::setErrorCallback([](const std::string& name, const std::string& error) {
        showNotification("Erreur de téléchargement: " + error, NotificationType::ERROR);
    });
    
    LOG_INFO("Interface utilisateur initialisée avec succès");
    return 0;
}

void MainWindow::cleanup() {
    LOG_INFO("Nettoyage de l'interface utilisateur...");
    
    // Libération des polices
    if (s_font_small) TTF_CloseFont(s_font_small);
    if (s_font_medium) TTF_CloseFont(s_font_medium);
    if (s_font_large) TTF_CloseFont(s_font_large);
    
    // Libération des ressources SDL
    if (s_renderer) SDL_DestroyRenderer(s_renderer);
    if (s_window) SDL_DestroyWindow(s_window);
    
    TTF_Quit();
    SDL_Quit();
    
    LOG_INFO("Interface utilisateur nettoyée");
}

bool MainWindow::handleEvents() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
                
            case SDL_KEYDOWN:
                return handleKeyPress(event.key.keysym.sym);
                
            case SDL_JOYBUTTONDOWN:
                return handleControllerButton(event.jbutton.button);
                
            default:
                break;
        }
    }
    
    return true;
}

void MainWindow::render() {
    // Effacement de l'écran
    SDL_SetRenderDrawColor(s_renderer, COLOR_BACKGROUND.r, COLOR_BACKGROUND.g, COLOR_BACKGROUND.b, COLOR_BACKGROUND.a);
    SDL_RenderClear(s_renderer);
    
    // Rendu selon l'état actuel
    switch (s_current_state) {
        case UIState::MAIN_MENU:
            renderMainMenu();
            break;
            
        case UIState::GAME_LIST:
            renderGameList();
            break;
            
        case UIState::DOWNLOADS:
            renderDownloads();
            break;
            
        case UIState::SETTINGS:
            renderSettings();
            break;
            
        case UIState::GAME_DETAILS:
            renderGameDetails();
            break;
    }
    
    // Rendu des notifications
    renderNotifications();
    
    // Présentation
    SDL_RenderPresent(s_renderer);
}

void MainWindow::updateGameList() {
    // Mise à jour de la liste des jeux disponibles
    // Dans un vrai projet, ceci viendrait du réseau P2P
    LOG_DEBUG("Mise à jour de la liste des jeux");
}

void MainWindow::updateDownloadProgress(const std::string& name, float progress) {
    // Mise à jour du progrès de téléchargement
    for (auto& download : s_active_downloads) {
        if (download.name == name) {
            download.progress = progress;
            break;
        }
    }
}

void MainWindow::showNotification(const std::string& message, NotificationType type) {
    s_notification_text = message;
    s_notification_time = Utils::getCurrentTimestamp();
    
    LOG_INFO("Notification: " + message);
}

// Méthodes privées
bool MainWindow::loadFonts() {
    // Chargement des polices (utilisation d'une police système par défaut)
    // Sur PS4, il faudrait utiliser les polices système disponibles
    
    const char* font_path = "/system/fonts/SCE-PS3-RD-R-LATIN.TTF"; // Police PS4 fictive
    
    // Fallback vers une police par défaut si la police PS4 n'est pas disponible
    if (!Utils::fileExists(font_path)) {
        font_path = nullptr; // SDL utilisera une police par défaut
    }
    
    s_font_large = TTF_OpenFont(font_path, 32);
    s_font_medium = TTF_OpenFont(font_path, 24);
    s_font_small = TTF_OpenFont(font_path, 16);
    
    if (!s_font_large || !s_font_medium || !s_font_small) {
        LOG_ERROR("Erreur de chargement des polices: " + std::string(TTF_GetError()));
        return false;
    }
    
    return true;
}

void MainWindow::initializeTestData() {
    // Données de test pour le développement
    s_available_games.clear();
    
    GameInfo game1;
    game1.title = "Horizon Zero Dawn";
    game1.description = "Action RPG dans un monde post-apocalyptique";
    game1.size_mb = 67000;
    game1.seeders = 45;
    game1.leechers = 12;
    game1.magnet_link = "magnet:?xt=urn:btih:example1";
    s_available_games.push_back(game1);
    
    GameInfo game2;
    game2.title = "The Last of Us Part II";
    game2.description = "Jeu d'action-aventure post-apocalyptique";
    game2.size_mb = 78000;
    game2.seeders = 67;
    game2.leechers = 23;
    game2.magnet_link = "magnet:?xt=urn:btih:example2";
    s_available_games.push_back(game2);
    
    GameInfo game3;
    game3.title = "God of War";
    game3.description = "Action-aventure mythologique nordique";
    game3.size_mb = 45000;
    game3.seeders = 89;
    game3.leechers = 34;
    game3.magnet_link = "magnet:?xt=urn:btih:example3";
    s_available_games.push_back(game3);
}

bool MainWindow::handleKeyPress(SDL_Keycode key) {
    switch (key) {
        case SDLK_ESCAPE:
        case SDLK_q:
            return false; // Quitter
            
        case SDLK_UP:
            navigateUp();
            break;
            
        case SDLK_DOWN:
            navigateDown();
            break;
            
        case SDLK_LEFT:
            navigateLeft();
            break;
            
        case SDLK_RIGHT:
            navigateRight();
            break;
            
        case SDLK_RETURN:
        case SDLK_SPACE:
            handleSelect();
            break;
            
        case SDLK_BACKSPACE:
            handleBack();
            break;
            
        default:
            break;
    }
    
    return true;
}

bool MainWindow::handleControllerButton(Uint8 button) {
    // Mapping des boutons de manette PS4
    switch (button) {
        case 0: // X (Croix)
            handleSelect();
            break;
            
        case 1: // Cercle
            handleBack();
            break;
            
        case 2: // Carré
            // Action secondaire
            break;
            
        case 3: // Triangle
            // Menu contextuel
            break;
            
        case 9: // Options
            return false; // Quitter
            
        default:
            break;
    }
    
    return true;
}

void MainWindow::navigateUp() {
    if (s_selected_index > 0) {
        s_selected_index--;
        
        // Ajustement du scroll si nécessaire
        if (s_selected_index < s_scroll_offset) {
            s_scroll_offset = s_selected_index;
        }
    }
}

void MainWindow::navigateDown() {
    int max_items = 0;
    
    switch (s_current_state) {
        case UIState::MAIN_MENU:
            max_items = 4; // Nombre d'options du menu principal
            break;
        case UIState::GAME_LIST:
            max_items = s_available_games.size();
            break;
        case UIState::DOWNLOADS:
            max_items = s_active_downloads.size();
            break;
        default:
            return;
    }
    
    if (s_selected_index < max_items - 1) {
        s_selected_index++;
        
        // Ajustement du scroll si nécessaire
        const int visible_items = 8; // Nombre d'éléments visibles
        if (s_selected_index >= s_scroll_offset + visible_items) {
            s_scroll_offset = s_selected_index - visible_items + 1;
        }
    }
}

void MainWindow::navigateLeft() {
    // Navigation horizontale si nécessaire
}

void MainWindow::navigateRight() {
    // Navigation horizontale si nécessaire
}

void MainWindow::handleSelect() {
    switch (s_current_state) {
        case UIState::MAIN_MENU:
            handleMainMenuSelect();
            break;
            
        case UIState::GAME_LIST:
            handleGameListSelect();
            break;
            
        case UIState::DOWNLOADS:
            handleDownloadsSelect();
            break;
            
        default:
            break;
    }
}

void MainWindow::handleBack() {
    switch (s_current_state) {
        case UIState::MAIN_MENU:
            // Quitter l'application
            break;
            
        case UIState::GAME_LIST:
        case UIState::DOWNLOADS:
        case UIState::SETTINGS:
            s_current_state = UIState::MAIN_MENU;
            s_selected_index = 0;
            s_scroll_offset = 0;
            break;
            
        case UIState::GAME_DETAILS:
            s_current_state = UIState::GAME_LIST;
            break;
    }
}

void MainWindow::handleMainMenuSelect() {
    switch (s_selected_index) {
        case 0: // Parcourir les jeux
            s_current_state = UIState::GAME_LIST;
            s_selected_index = 0;
            s_scroll_offset = 0;
            break;
            
        case 1: // Téléchargements
            s_current_state = UIState::DOWNLOADS;
            s_selected_index = 0;
            s_scroll_offset = 0;
            updateDownloadsList();
            break;
            
        case 2: // Paramètres
            s_current_state = UIState::SETTINGS;
            s_selected_index = 0;
            s_scroll_offset = 0;
            break;
            
        case 3: // Quitter
            // Géré par le retour false
            break;
    }
}

void MainWindow::handleGameListSelect() {
    if (s_selected_index < s_available_games.size()) {
        s_current_state = UIState::GAME_DETAILS;
    }
}

void MainWindow::handleDownloadsSelect() {
    // Actions sur les téléchargements (pause, reprise, annulation)
    if (s_selected_index < s_active_downloads.size()) {
        const auto& download = s_active_downloads[s_selected_index];
        
        if (download.is_finished) {
            // Installer le package
            showNotification("Installation de " + download.name + "...", NotificationType::INFO);
            PkgManager::installPackage(download.save_path + "/" + download.name + ".pkg");
        } else {
            // Pause/reprise du téléchargement
            TorrentManager::stopDownload(download.name);
            showNotification("Téléchargement mis en pause: " + download.name, NotificationType::INFO);
        }
    }
}

void MainWindow::updateDownloadsList() {
    s_active_downloads = TorrentManager::getDownloads();
}

void MainWindow::renderMainMenu() {
    const char* menu_items[] = {
        "Parcourir les jeux",
        "Téléchargements",
        "Paramètres",
        "Quitter"
    };
    
    // Titre
    renderText("PS4 Store P2P", 960, 200, s_font_large, COLOR_PRIMARY, true);
    renderText("Store décentralisé pour PS4 jailbreakée", 960, 250, s_font_medium, COLOR_TEXT_SECONDARY, true);
    
    // Menu
    for (int i = 0; i < 4; i++) {
        SDL_Color color = (i == s_selected_index) ? COLOR_PRIMARY : COLOR_TEXT;
        int y = 400 + i * 60;
        
        if (i == s_selected_index) {
            // Surbrillance
            SDL_Rect rect = {760, y - 10, 400, 40};
            SDL_SetRenderDrawColor(s_renderer, COLOR_PRIMARY.r, COLOR_PRIMARY.g, COLOR_PRIMARY.b, 50);
            SDL_RenderFillRect(s_renderer, &rect);
        }
        
        renderText(menu_items[i], 960, y, s_font_medium, color, true);
    }
    
    // Instructions
    renderText("Utilisez les flèches pour naviguer, Entrée pour sélectionner", 960, 800, s_font_small, COLOR_TEXT_SECONDARY, true);
}

void MainWindow::renderGameList() {
    // Titre
    renderText("Jeux disponibles", 100, 50, s_font_large, COLOR_TEXT);
    
    // Liste des jeux
    const int items_per_page = 8;
    const int item_height = 100;
    
    for (int i = 0; i < items_per_page && (s_scroll_offset + i) < s_available_games.size(); i++) {
        int game_index = s_scroll_offset + i;
        const GameInfo& game = s_available_games[game_index];
        
        int y = 150 + i * item_height;
        bool is_selected = (game_index == s_selected_index);
        
        // Arrière-plan de sélection
        if (is_selected) {
            SDL_Rect rect = {50, y - 10, 1820, item_height - 20};
            SDL_SetRenderDrawColor(s_renderer, COLOR_PRIMARY.r, COLOR_PRIMARY.g, COLOR_PRIMARY.b, 100);
            SDL_RenderFillRect(s_renderer, &rect);
        }
        
        // Informations du jeu
        SDL_Color title_color = is_selected ? COLOR_PRIMARY : COLOR_TEXT;
        renderText(game.title, 100, y, s_font_medium, title_color);
        renderText(game.description, 100, y + 30, s_font_small, COLOR_TEXT_SECONDARY);
        
        // Taille et seeders
        std::string size_text = Utils::formatFileSize(game.size_mb * 1024 * 1024);
        std::string seeders_text = "Seeders: " + std::to_string(game.seeders) + " | Leechers: " + std::to_string(game.leechers);
        
        renderText(size_text, 1500, y, s_font_small, COLOR_TEXT_SECONDARY);
        renderText(seeders_text, 1500, y + 25, s_font_small, COLOR_TEXT_SECONDARY);
    }
    
    // Instructions
    renderText("Entrée: Détails | Retour: Menu principal", 100, 1000, s_font_small, COLOR_TEXT_SECONDARY);
}

void MainWindow::renderDownloads() {
    // Titre
    renderText("Téléchargements actifs", 100, 50, s_font_large, COLOR_TEXT);
    
    if (s_active_downloads.empty()) {
        renderText("Aucun téléchargement en cours", 960, 400, s_font_medium, COLOR_TEXT_SECONDARY, true);
        return;
    }
    
    // Liste des téléchargements
    const int item_height = 120;
    
    for (size_t i = 0; i < s_active_downloads.size(); i++) {
        const DownloadInfo& download = s_active_downloads[i];
        
        int y = 150 + i * item_height;
        bool is_selected = (i == s_selected_index);
        
        // Arrière-plan de sélection
        if (is_selected) {
            SDL_Rect rect = {50, y - 10, 1820, item_height - 20};
            SDL_SetRenderDrawColor(s_renderer, COLOR_PRIMARY.r, COLOR_PRIMARY.g, COLOR_PRIMARY.b, 100);
            SDL_RenderFillRect(s_renderer, &rect);
        }
        
        // Nom du téléchargement
        SDL_Color title_color = is_selected ? COLOR_PRIMARY : COLOR_TEXT;
        renderText(download.name, 100, y, s_font_medium, title_color);
        
        // Statut
        renderText(download.status, 100, y + 30, s_font_small, COLOR_TEXT_SECONDARY);
        
        // Barre de progression
        SDL_Rect progress_bg = {100, y + 60, 800, 20};
        SDL_Rect progress_fill = {100, y + 60, static_cast<int>(800 * download.progress), 20};
        
        SDL_SetRenderDrawColor(s_renderer, COLOR_SECONDARY.r, COLOR_SECONDARY.g, COLOR_SECONDARY.b, 255);
        SDL_RenderFillRect(s_renderer, &progress_bg);
        
        SDL_SetRenderDrawColor(s_renderer, COLOR_SUCCESS.r, COLOR_SUCCESS.g, COLOR_SUCCESS.b, 255);
        SDL_RenderFillRect(s_renderer, &progress_fill);
        
        // Pourcentage et vitesse
        std::string progress_text = Utils::formatPercentage(download.progress) + " - " + 
                                   Utils::formatSpeed(download.download_rate);
        renderText(progress_text, 920, y + 60, s_font_small, COLOR_TEXT);
        
        // Taille
        std::string size_text = Utils::formatFileSize(download.downloaded) + " / " + 
                               Utils::formatFileSize(download.total_size);
        renderText(size_text, 1400, y + 30, s_font_small, COLOR_TEXT_SECONDARY);
    }
    
    // Instructions
    renderText("Entrée: Pause/Reprise | Retour: Menu principal", 100, 1000, s_font_small, COLOR_TEXT_SECONDARY);
}

void MainWindow::renderSettings() {
    renderText("Paramètres", 100, 50, s_font_large, COLOR_TEXT);
    renderText("Fonctionnalité en développement...", 960, 400, s_font_medium, COLOR_TEXT_SECONDARY, true);
    renderText("Retour: Menu principal", 100, 1000, s_font_small, COLOR_TEXT_SECONDARY);
}

void MainWindow::renderGameDetails() {
    if (s_selected_index >= s_available_games.size()) {
        return;
    }
    
    const GameInfo& game = s_available_games[s_selected_index];
    
    // Titre du jeu
    renderText(game.title, 100, 50, s_font_large, COLOR_TEXT);
    
    // Description
    renderText("Description:", 100, 150, s_font_medium, COLOR_TEXT);
    renderText(game.description, 100, 190, s_font_small, COLOR_TEXT_SECONDARY);
    
    // Informations
    renderText("Taille: " + Utils::formatFileSize(game.size_mb * 1024 * 1024), 100, 250, s_font_small, COLOR_TEXT);
    renderText("Seeders: " + std::to_string(game.seeders), 100, 280, s_font_small, COLOR_TEXT);
    renderText("Leechers: " + std::to_string(game.leechers), 100, 310, s_font_small, COLOR_TEXT);
    
    // Bouton de téléchargement
    SDL_Rect download_btn = {100, 400, 300, 60};
    SDL_SetRenderDrawColor(s_renderer, COLOR_SUCCESS.r, COLOR_SUCCESS.g, COLOR_SUCCESS.b, 255);
    SDL_RenderFillRect(s_renderer, &download_btn);
    
    renderText("Télécharger", 250, 420, s_font_medium, COLOR_TEXT, true);
    
    // Instructions
    renderText("Entrée: Télécharger | Retour: Liste des jeux", 100, 1000, s_font_small, COLOR_TEXT_SECONDARY);
}

void MainWindow::renderNotifications() {
    if (s_notification_text.empty()) {
        return;
    }
    
    int64_t current_time = Utils::getCurrentTimestamp();
    if (current_time - s_notification_time > NOTIFICATION_DURATION) {
        s_notification_text.clear();
        return;
    }
    
    // Calcul de l'opacité basée sur le temps restant
    float time_ratio = static_cast<float>(current_time - s_notification_time) / NOTIFICATION_DURATION;
    int alpha = static_cast<int>(255 * (1.0f - time_ratio));
    
    // Arrière-plan de la notification
    SDL_Rect notification_rect = {400, 50, 1120, 80};
    SDL_SetRenderDrawColor(s_renderer, 0, 0, 0, alpha / 2);
    SDL_RenderFillRect(s_renderer, &notification_rect);
    
    // Texte de la notification
    SDL_Color notification_color = COLOR_TEXT;
    notification_color.a = alpha;
    renderText(s_notification_text, 960, 90, s_font_medium, notification_color, true);
}

void MainWindow::renderText(const std::string& text, int x, int y, TTF_Font* font, 
                           const SDL_Color& color, bool centered) {
    if (!font || text.empty()) {
        return;
    }
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        return;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(s_renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    int text_width = surface->w;
    int text_height = surface->h;
    
    SDL_FreeSurface(surface);
    
    SDL_Rect dest_rect;
    dest_rect.x = centered ? x - text_width / 2 : x;
    dest_rect.y = y;
    dest_rect.w = text_width;
    dest_rect.h = text_height;
    
    SDL_RenderCopy(s_renderer, texture, nullptr, &dest_rect);
    SDL_DestroyTexture(texture);
}