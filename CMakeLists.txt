cmake_minimum_required(VERSION 3.16)
project(PS4StoreP2P VERSION 1.0.0 LANGUAGES C CXX)

# Configuration du projet
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Variables de configuration
set(PROJECT_NAME "ps4_store_p2p")
set(OPENORBIS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/OpenOrbis-PS4-Toolchain" CACHE PATH "Path to OpenOrbis Toolchain")
set(LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

# Détection de la plateforme
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    set(PS4_BUILD ON)
    message(STATUS "Building for PS4")
else()
    set(PS4_BUILD OFF)
    message(STATUS "Building for host platform (development/testing)")
endif()

# Configuration pour PS4
if(PS4_BUILD)
    # Flags spécifiques PS4
    set(PS4_FLAGS "-target x86_64-pc-freebsd12-elf -fPIC -funwind-tables")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PS4_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PS4_FLAGS}")
    
    # Définitions pour PS4
    add_definitions(-D__ORBIS__ -D__PS4__)
    
    # Chemins d'inclusion OpenOrbis
    if(EXISTS "${OPENORBIS_ROOT}")
        include_directories("${OPENORBIS_ROOT}/include")
        link_directories("${OPENORBIS_ROOT}/lib")
    else()
        message(WARNING "OpenOrbis Toolchain not found at ${OPENORBIS_ROOT}")
    endif()
else()
    # Configuration pour le développement sur PC
    add_definitions(-DDEV_BUILD)
endif()

# Répertoires d'inclusion
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/p2p
    ${CMAKE_CURRENT_SOURCE_DIR}/include/pkg
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
)

# Configuration SDL2
if(PS4_BUILD)
    # SDL2 compilé pour PS4
    set(SDL2_DIR "${LIBS_DIR}/SDL2")
    if(EXISTS "${SDL2_DIR}")
        include_directories("${SDL2_DIR}/include")
        link_directories("${SDL2_DIR}/build")
        set(SDL2_LIBRARIES SDL2 SDL2main)
    else()
        message(WARNING "SDL2 for PS4 not found at ${SDL2_DIR}")
    endif()
else()
    # SDL2 système pour le développement
    find_package(SDL2 REQUIRED)
    if(SDL2_FOUND)
        include_directories(${SDL2_INCLUDE_DIRS})
        set(SDL2_LIBRARIES ${SDL2_LIBRARIES})
    endif()
endif()

# Configuration ImGui
set(IMGUI_DIR "${LIBS_DIR}/imgui")
if(EXISTS "${IMGUI_DIR}")
    include_directories("${IMGUI_DIR}")
    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
    )
else()
    message(WARNING "ImGui not found at ${IMGUI_DIR}")
    set(IMGUI_SOURCES "")
endif()

# Configuration libtorrent
if(PS4_BUILD)
    # libtorrent compilé pour PS4
    set(LIBTORRENT_DIR "${LIBS_DIR}/libtorrent")
    if(EXISTS "${LIBTORRENT_DIR}")
        include_directories("${LIBTORRENT_DIR}/include")
        link_directories("${LIBTORRENT_DIR}/build")
        set(LIBTORRENT_LIBRARIES torrent-rasterbar)
    else()
        message(WARNING "libtorrent for PS4 not found at ${LIBTORRENT_DIR}")
    endif()
else()
    # libtorrent système pour le développement
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBTORRENT libtorrent-rasterbar)
        if(LIBTORRENT_FOUND)
            include_directories(${LIBTORRENT_INCLUDE_DIRS})
            link_directories(${LIBTORRENT_LIBRARY_DIRS})
            set(LIBTORRENT_LIBRARIES ${LIBTORRENT_LIBRARIES})
        endif()
    endif()
endif()

# Sources du projet
set(SOURCES
    src/main.cpp
    src/ui/main_window.cpp
    src/p2p/torrent_manager.cpp
    src/pkg/pkg_manager.cpp
    src/utils/utils.cpp
)

# Headers du projet
set(HEADERS
    include/ui/main_window.h
    include/p2p/torrent_manager.h
    include/pkg/pkg_manager.h
    include/utils/utils.h
)

# Création de l'exécutable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${IMGUI_SOURCES})

# Liaison des bibliothèques
target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${LIBTORRENT_LIBRARIES}
)

# Bibliothèques système spécifiques à PS4
if(PS4_BUILD)
    target_link_libraries(${PROJECT_NAME}
        SceLibcInternal
        SceNet
        SceNetCtl
        SceSysmodule
        SceUserService
        SceSystemService
        ScePad
        SceAudioOut
        SceVideoOut
        kernel
        c
        m
    )
else()
    # Bibliothèques système pour le développement
    if(WIN32)
        target_link_libraries(${PROJECT_NAME} ws2_32 winmm)
    elseif(UNIX)
        target_link_libraries(${PROJECT_NAME} pthread dl)
    endif()
endif()

# Propriétés de l'exécutable
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Configuration spécifique PS4
if(PS4_BUILD)
    # Extension .elf pour PS4
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")
    
    # Flags de liaison PS4
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-Wl,--oformat=elf64-x86-64 -Wl,-z,max-page-size=0x4000"
    )
endif()

# Cibles personnalisées
if(PS4_BUILD)
    # Cible pour créer le package PKG
    add_custom_target(pkg
        COMMAND orbis-pub-gen ${CMAKE_CURRENT_SOURCE_DIR}/project.gp4 ${CMAKE_BINARY_DIR}/
        DEPENDS ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Creating PS4 PKG package"
    )
    
    # Cible pour installer via FTP (nécessite une PS4 connectée)
    add_custom_target(install_ftp
        COMMAND echo "Upload ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pkg to PS4 via FTP"
        DEPENDS pkg
        COMMENT "Installing PKG to PS4 via FTP"
    )
endif()

# Cible pour nettoyer complètement
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Removing all build files"
)

# Configuration de débogage
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_BUILD)
    if(NOT PS4_BUILD)
        # Flags de débogage pour le développement
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -Wextra")
    endif()
else()
    add_definitions(-DRELEASE_BUILD)
    if(NOT PS4_BUILD)
        # Optimisations pour la release
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG")
    endif()
endif()

# Affichage des informations de configuration
message(STATUS "=== Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "PS4 Build: ${PS4_BUILD}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Source directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Binary directory: ${CMAKE_BINARY_DIR}")
if(PS4_BUILD)
    message(STATUS "OpenOrbis Root: ${OPENORBIS_ROOT}")
endif()
message(STATUS "=============================")

# Installation (pour make install)
if(PS4_BUILD)
    install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf
        DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    )
    
    if(EXISTS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pkg)
        install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pkg
            DESTINATION packages
        )
    endif()
else()
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
    )
endif()

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()