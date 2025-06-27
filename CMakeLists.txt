cmake_minimum_required(VERSION 3.16)
project(PS4StoreP2P VERSION 1.0.0 LANGUAGES C CXX)

# Configuration du projet - PS4 ONLY
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Variables de configuration
set(PROJECT_NAME "ps4_store_p2p")
set(OPENORBIS_ROOT "/opt/OpenOrbis" CACHE PATH "Path to OpenOrbis Toolchain")
set(LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")

# PS4 Build Only - No cross-platform support
set(PS4_BUILD ON)
message(STATUS "Building for PS4 only")

# Configuration PS4
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
    message(FATAL_ERROR "OpenOrbis Toolchain not found at ${OPENORBIS_ROOT}. Please install OpenOrbis first.")
endif()

# Répertoires d'inclusion
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/p2p
    ${CMAKE_CURRENT_SOURCE_DIR}/include/pkg
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
)

# Configuration SDL2 pour PS4
set(SDL2_DIR "${LIBS_DIR}/SDL2-2.28.5")
message(STATUS "Using PS4 SDL2 at: ${SDL2_DIR}")
if(EXISTS "${SDL2_DIR}")
    include_directories("${SDL2_DIR}/include")
    # Use SDL2 cmake config files
    set(CMAKE_PREFIX_PATH "${SDL2_DIR}/cmake" ${CMAKE_PREFIX_PATH})
    find_package(SDL2 REQUIRED CONFIG PATHS "${SDL2_DIR}/cmake" NO_DEFAULT_PATH)
    set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
else()
    message(FATAL_ERROR "SDL2 for PS4 not found at ${SDL2_DIR}. Please install SDL2 for PS4.")
endif()


# Configuration ImGui pour PS4
set(IMGUI_DIR "${LIBS_DIR}/imgui-1.90.1")
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
    message(STATUS "Including ImGui SDL2 backends for PS4")
else()
    message(FATAL_ERROR "ImGui not found at ${IMGUI_DIR}. Please install ImGui.")
endif()

# Configuration libtorrent pour PS4
set(LIBTORRENT_DIR "${LIBS_DIR}/libtorrent")
if(EXISTS "${LIBTORRENT_DIR}")
    include_directories("${LIBTORRENT_DIR}/include")
    link_directories("${LIBTORRENT_DIR}/build")
    set(LIBTORRENT_LIBRARIES torrent-rasterbar)
else()
    message(FATAL_ERROR "libtorrent not found at ${LIBTORRENT_DIR}. Please install libtorrent for PS4.")
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

# Liaison des bibliothèques pour PS4
target_link_libraries(${PROJECT_NAME} 
    ${SDL2_LIBRARIES}
    ${LIBTORRENT_LIBRARIES}
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

# Propriétés de l'exécutable PS4
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    SUFFIX ".elf"
    LINK_FLAGS "-Wl,--oformat=elf64-x86-64 -Wl,-z,max-page-size=0x4000"
)

# Cibles personnalisées PS4
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

# Cible pour nettoyer complètement
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Removing all build files"
)

# Configuration de débogage PS4
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_BUILD)
else()
    add_definitions(-DRELEASE_BUILD)
endif()

# Affichage des informations de configuration PS4
message(STATUS "=== PS4 Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target platform: PS4")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Source directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Binary directory: ${CMAKE_BINARY_DIR}")
message(STATUS "OpenOrbis Root: ${OPENORBIS_ROOT}")
message(STATUS "====================================")

# Installation PS4 (pour make install)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)

if(EXISTS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pkg)
    install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pkg
        DESTINATION packages
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