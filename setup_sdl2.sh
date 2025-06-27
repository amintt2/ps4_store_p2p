#!/bin/bash
# Quick setup script for SDL2 PS4 build
# This script helps fix the SDL2 configuration issue

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_status "Setting up SDL2 for PS4 development..."

# Check if OpenOrbis toolchain is available
if [ ! -d "OpenOrbis-PS4-Toolchain" ]; then
    print_error "OpenOrbis toolchain not found. Please run './extract_openorbis.sh' first."
    exit 1
fi

# Create libs directory if it doesn't exist
mkdir -p libs

# Check if we have the source SDL2
if [ -d "libs/SDL2-2.28.5" ] && [ ! -d "libs/SDL2" ]; then
    print_status "Found SDL2 source, setting up for PS4 build..."
    
    # Copy source to build directory
    cp -r "libs/SDL2-2.28.5" "libs/SDL2"
    
    cd "libs/SDL2"
    
    # Create build directory
    mkdir -p build
    cd build
    
    print_status "Configuring SDL2 for PS4..."
    
    # Set up environment for PS4 build
    export OPENORBIS_ROOT="$(pwd)/../../../OpenOrbis-PS4-Toolchain"
    export PATH="$OPENORBIS_ROOT/bin:$PATH"
    
    # Configure with CMake for PS4
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE="$OPENORBIS_ROOT/cmake/toolchain.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DSDL_SHARED=OFF \
        -DSDL_STATIC=ON \
        -DSDL_TEST=OFF \
        -DSDL_TESTS=OFF
    
    print_status "Building SDL2 for PS4..."
    make -j$(nproc)
    
    cd ../..
    
    print_success "SDL2 for PS4 built successfully!"
    print_status "You can now run 'cmake ..' in the build directory."
else
    print_warning "SDL2 setup already exists or source not found."
    if [ -d "libs/SDL2" ]; then
        print_status "SDL2 build directory already exists at libs/SDL2"
    fi
fi

print_success "SDL2 setup complete!"