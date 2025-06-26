/**
 * PS4 Store P2P - Interface Utilisateur Principale
 * 
 * Gère l'interface utilisateur principale avec SDL2
 * Affiche la liste des jeux, les options de téléchargement, etc.
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>

// Structure pour représenter un jeu dans le store
struct GameInfo {
    std::string title;
    std::string description;
    std::string version;
    std::string size;
    std::string magnet_link;
    std::string icon_path;
    bool is_downloading;
    float download_progress;
    int seeders;
    int leechers;
};

// États de l'interface
enum class UIState {
    MAIN_MENU,
    GAME_LIST,
    GAME_DETAILS,
    DOWNLOADS,
    SETTINGS,
    ABOUT
};

class MainWindow {
public:
    /**
     * Initialise l'interface principale
     * @param renderer Le renderer SDL à utiliser
     * @return 0 en cas de succès, -1 en cas d'erreur
     */
    static int initialize(SDL_Renderer* renderer);
    
    /**
     * Nettoie les ressources de l'interface
     */
    static void cleanup();
    
    /**
     * Gère les événements SDL
     * @param event L'événement à traiter
     */
    static void handleEvent(const SDL_Event& event);
    
    /**
     * Rend l'interface à l'écran
     * @param renderer Le renderer SDL à utiliser
     */
    static void render(SDL_Renderer* renderer);
    
    /**
     * Met à jour la liste des jeux
     * @param games Liste des jeux à afficher
     */
    static void updateGameList(const std::vector<GameInfo>& games);
    
    /**
     * Met à jour le progrès de téléchargement d'un jeu
     * @param game_title Titre du jeu
     * @param progress Progrès (0.0 à 1.0)
     */
    static void updateDownloadProgress(const std::string& game_title, float progress);
    
    /**
     * Affiche une notification
     * @param message Message à afficher
     * @param type Type de notification (info, warning, error)
     */
    static void showNotification(const std::string& message, const std::string& type = "info");

private:
    static SDL_Renderer* s_renderer;
    static TTF_Font* s_font_large;
    static TTF_Font* s_font_medium;
    static TTF_Font* s_font_small;
    
    static UIState s_current_state;
    static std::vector<GameInfo> s_games;
    static int s_selected_game;
    static int s_scroll_offset;
    
    static std::string s_notification_message;
    static std::string s_notification_type;
    static Uint32 s_notification_time;
    
    // Méthodes de rendu pour chaque état
    static void renderMainMenu();
    static void renderGameList();
    static void renderGameDetails();
    static void renderDownloads();
    static void renderSettings();
    static void renderAbout();
    
    // Méthodes utilitaires
    static void renderText(const std::string& text, int x, int y, TTF_Font* font, SDL_Color color);
    static void renderButton(const std::string& text, int x, int y, int width, int height, bool selected = false);
    static void renderProgressBar(int x, int y, int width, int height, float progress);
    static void renderGameCard(const GameInfo& game, int x, int y, int width, int height, bool selected = false);
    static void renderNotification();
    
    // Gestion des événements pour chaque état
    static void handleMainMenuEvent(const SDL_Event& event);
    static void handleGameListEvent(const SDL_Event& event);
    static void handleGameDetailsEvent(const SDL_Event& event);
    static void handleDownloadsEvent(const SDL_Event& event);
    static void handleSettingsEvent(const SDL_Event& event);
    static void handleAboutEvent(const SDL_Event& event);
};

#endif // MAIN_WINDOW_H