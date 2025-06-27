# Toolchain file for PS4 cross-compilation using OpenOrbis
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Find OpenOrbis toolchain
set(OPENORBIS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/OpenOrbis-PS4-Toolchain" CACHE PATH "Path to OpenOrbis Toolchain")

if(NOT EXISTS "${OPENORBIS_ROOT}")
    message(FATAL_ERROR "OpenOrbis toolchain not found at ${OPENORBIS_ROOT}. Please run extract_openorbis.sh first.")
endif()

# Set compilers
set(CMAKE_C_COMPILER "${OPENORBIS_ROOT}/bin/orbis-clang")
set(CMAKE_CXX_COMPILER "${OPENORBIS_ROOT}/bin/orbis-clang++")
set(CMAKE_AR "${OPENORBIS_ROOT}/bin/orbis-ar")
set(CMAKE_RANLIB "${OPENORBIS_ROOT}/bin/orbis-ranlib")

# Set sysroot
set(CMAKE_SYSROOT "${OPENORBIS_ROOT}")
set(CMAKE_FIND_ROOT_PATH "${OPENORBIS_ROOT}")

# PS4 specific flags
set(PS4_FLAGS "-target x86_64-pc-freebsd12-elf -fPIC -funwind-tables")
set(CMAKE_C_FLAGS "${PS4_FLAGS}")
set(CMAKE_CXX_FLAGS "${PS4_FLAGS} -std=c++17")

# Include and library paths
include_directories("${OPENORBIS_ROOT}/include")
link_directories("${OPENORBIS_ROOT}/lib")

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Disable compiler checks
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR "")

# PS4 definitions
add_definitions(-D__ORBIS__ -D__PS4__)