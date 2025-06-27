#!/bin/bash

# OpenOrbis PS4 Toolchain Extraction Script for Linux
# This script downloads and extracts the OpenOrbis toolchain

set -e  # Exit on any error

# Configuration
OPENORBIS_DIR="OpenOrbis-PS4-Toolchain"
TOOLCHAIN_URL="https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain/releases/download/v0.5.3/toolchain-llvm-12.zip"
TOOLCHAIN_ZIP="toolchain-llvm-12.zip"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
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

# Check prerequisites
check_prerequisites() {
    print_status "Checking prerequisites..."
    
    # Check if wget or curl is available
    if ! command -v wget &> /dev/null && ! command -v curl &> /dev/null; then
        print_error "Neither wget nor curl is available. Please install one of them."
        exit 1
    fi
    
    # Check if unzip is available
    if ! command -v unzip &> /dev/null; then
        print_error "unzip is not available. Please install unzip."
        exit 1
    fi
    
    print_success "Prerequisites check passed"
}

# Download toolchain
download_toolchain() {
    print_status "Downloading OpenOrbis toolchain..."
    
    if [ -f "$TOOLCHAIN_ZIP" ]; then
        print_warning "$TOOLCHAIN_ZIP already exists. Removing old file..."
        rm "$TOOLCHAIN_ZIP"
    fi
    
    if command -v wget &> /dev/null; then
        wget -O "$TOOLCHAIN_ZIP" "$TOOLCHAIN_URL"
    elif command -v curl &> /dev/null; then
        curl -L -o "$TOOLCHAIN_ZIP" "$TOOLCHAIN_URL"
    fi
    
    if [ ! -f "$TOOLCHAIN_ZIP" ]; then
        print_error "Failed to download toolchain"
        exit 1
    fi
    
    print_success "Toolchain downloaded successfully"
}

# Extract toolchain
extract_toolchain() {
    print_status "Extracting OpenOrbis toolchain..."
    
    if [ -d "$OPENORBIS_DIR" ]; then
        print_warning "$OPENORBIS_DIR already exists. Removing old directory..."
        rm -rf "$OPENORBIS_DIR"
    fi
    
    mkdir -p "$OPENORBIS_DIR"
    unzip -q "$TOOLCHAIN_ZIP" -d "$OPENORBIS_DIR"
    
    # Check if extraction was successful
    if [ ! -d "$OPENORBIS_DIR" ] || [ -z "$(ls -A $OPENORBIS_DIR)" ]; then
        print_error "Failed to extract toolchain"
        exit 1
    fi
    
    print_success "Toolchain extracted to $OPENORBIS_DIR"
}

# Cleanup
cleanup() {
    print_status "Cleaning up..."
    
    if [ -f "$TOOLCHAIN_ZIP" ]; then
        rm "$TOOLCHAIN_ZIP"
        print_status "Removed $TOOLCHAIN_ZIP"
    fi
    
    # Clean up empty temp directories
    if [ -d "temp_extract" ] && [ -z "$(ls -A temp_extract)" ]; then
        rmdir temp_extract
        print_status "Removed empty temp_extract directory"
    fi
    
    if [ -d "temp_extract2" ] && [ -z "$(ls -A temp_extract2)" ]; then
        rmdir temp_extract2
        print_status "Removed empty temp_extract2 directory"
    fi
}

# Set environment variables
setup_environment() {
    print_status "Setting up environment..."
    
    OPENORBIS_ROOT="$(pwd)/$OPENORBIS_DIR"
    
    echo "export OO_PS4_TOOLCHAIN=\"$OPENORBIS_ROOT\"" >> ~/.bashrc
    echo "export OPENORBIS_ROOT=\"$OPENORBIS_ROOT\"" >> ~/.bashrc
    
    print_success "Environment variables added to ~/.bashrc"
    print_warning "Please run 'source ~/.bashrc' or restart your terminal to apply changes"
}

# Verify installation
verify_installation() {
    print_status "Verifying installation..."
    
    if [ -d "$OPENORBIS_DIR/include" ] && [ -d "$OPENORBIS_DIR/lib" ]; then
        print_success "OpenOrbis toolchain installation verified"
        print_status "Toolchain location: $(pwd)/$OPENORBIS_DIR"
    else
        print_error "Installation verification failed"
        exit 1
    fi
}

# Main execution
main() {
    print_status "Starting OpenOrbis PS4 Toolchain extraction..."
    
    check_prerequisites
    download_toolchain
    extract_toolchain
    cleanup
    setup_environment
    verify_installation
    
    print_success "OpenOrbis toolchain extraction completed successfully!"
    print_status "You can now build your PS4 project with: cmake .. && make"
}

# Run main function
main "$@"