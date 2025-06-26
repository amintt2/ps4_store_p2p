/**
 * PS4 Store P2P - Store Décentralisé pour PS4 Jailbreakée
 * 
 * Point d'entrée principal de l'application
 * Utilise OpenOrbis SDK, SDL2 pour l'interface et libtorrent pour le P2P
 * 
 * @author PS4 Store P2P Team
 * @version 1.0.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Headers OpenOrbis
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include <orbis/SystemService.h>
#include <orbis/UserService.h>

// Headers SDL2 pour l'interface
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// Headers du projet
#include "ui/main_window.h"
#include "p2p/torrent_manager.h"
#include "pkg/pkg_manager.h"
#include "utils/logger.h"
#include "utils/config.h"

// Constantes
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define APP_NAME "PS4 Store P2P"
#define APP_VERSION "1.0.0"

// Variables globales
SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
bool g_running = true;

/**
 * Initialise les modules système PS4
 */
int initializePS4Systems() {
    int ret = 0;
    
    // Initialiser les modules système
    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    if (ret != 0) {
        printf("Erreur lors du chargement du module NET: 0x%08X\n", ret);
        return -1;
    }
    
    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
    if (ret != 0) {
        printf("Erreur lors du chargement du module HTTP: 0x%08X\n", ret);
        return -1;
    }
    
    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_SSL);
    if (ret != 0) {
        printf("Erreur lors du chargement du module SSL: 0x%08X\n", ret);
        return -1;
    }
    
    printf("Modules système PS4 initialisés avec succès\n");
    return 0;
}

/**
 * Initialise SDL2 et crée la fenêtre principale
 */
int initializeSDL() {
    // Initialiser SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return -1;
    }
    
    // Initialiser SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("Erreur IMG_Init: %s\n", IMG_GetError());
        return -1;
    }
    
    // Initialiser SDL_ttf
    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        return -1;
    }
    
    // Créer la fenêtre
    g_window = SDL_CreateWindow(
        APP_NAME,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN
    );
    
    if (!g_window) {
        printf("Erreur création fenêtre: %s\n", SDL_GetError());
        return -1;
    }
    
    // Créer le renderer
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        printf("Erreur création renderer: %s\n", SDL_GetError());
        return -1;
    }
    
    // Définir la couleur de fond
    SDL_SetRenderDrawColor(g_renderer, 0x1E, 0x1E, 0x2E, 0xFF);
    
    printf("SDL2 initialisé avec succès\n");
    return 0;
}

/**
 * Gère les événements SDL
 */
void handleEvents() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                g_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        g_running = false;
                        break;
                }
                break;
                
            case SDL_CONTROLLERBUTTONDOWN:
                switch (event.cbutton.button) {
                    case SDL_CONTROLLER_BUTTON_START:
                        g_running = false;
                        break;
                }
                break;
        }
        
        // Transmettre l'événement à l'interface
        MainWindow::handleEvent(event);
    }
}

/**
 * Boucle de rendu principal
 */
void render() {
    // Effacer l'écran
    SDL_SetRenderDrawColor(g_renderer, 0x1E, 0x1E, 0x2E, 0xFF);
    SDL_RenderClear(g_renderer);
    
    // Rendre l'interface principale
    MainWindow::render(g_renderer);
    
    // Présenter le rendu
    SDL_RenderPresent(g_renderer);
}

/**
 * Nettoie les ressources avant la fermeture
 */
void cleanup() {
    printf("Nettoyage des ressources...\n");
    
    // Nettoyer l'interface
    MainWindow::cleanup();
    
    // Nettoyer le gestionnaire P2P
    TorrentManager::cleanup();
    
    // Nettoyer le gestionnaire PKG
    PkgManager::cleanup();
    
    // Nettoyer SDL
    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = nullptr;
    }
    
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
    }
    
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    printf("Nettoyage terminé\n");
}

/**
 * Point d'entrée principal
 */
int main(int argc, char* argv[]) {
    printf("=== %s v%s ===\n", APP_NAME, APP_VERSION);
    printf("Démarrage de l'application...\n");
    
    // Initialiser le logger
    Logger::initialize("/data/ps4_store_p2p/logs/app.log");
    Logger::info("Application démarrée");
    
    // Charger la configuration
    if (Config::load("/data/ps4_store_p2p/config.json") != 0) {
        printf("Erreur lors du chargement de la configuration\n");
        return -1;
    }
    
    // Initialiser les systèmes PS4
    if (initializePS4Systems() != 0) {
        printf("Erreur lors de l'initialisation des systèmes PS4\n");
        return -1;
    }
    
    // Initialiser SDL
    if (initializeSDL() != 0) {
        printf("Erreur lors de l'initialisation de SDL\n");
        return -1;
    }
    
    // Initialiser les composants de l'application
    if (MainWindow::initialize(g_renderer) != 0) {
        printf("Erreur lors de l'initialisation de l'interface\n");
        cleanup();
        return -1;
    }
    
    if (TorrentManager::initialize() != 0) {
        printf("Erreur lors de l'initialisation du gestionnaire P2P\n");
        cleanup();
        return -1;
    }
    
    if (PkgManager::initialize() != 0) {
        printf("Erreur lors de l'initialisation du gestionnaire PKG\n");
        cleanup();
        return -1;
    }
    
    printf("Application initialisée avec succès\n");
    Logger::info("Application initialisée avec succès");
    
    // Boucle principale
    while (g_running) {
        handleEvents();
        render();
        
        // Mettre à jour les composants
        TorrentManager::update();
        PkgManager::update();
        
        // Limiter le framerate
        SDL_Delay(16); // ~60 FPS
    }
    
    printf("Fermeture de l'application...\n");
    Logger::info("Application fermée");
    
    cleanup();
    return 0;
}