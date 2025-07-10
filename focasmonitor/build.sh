#!/bin/bash

# FOCAS Monitor - Quick Build Script
# This script provides a one-command build for the FOCAS Monitor

set -e  # Exit on any error

# Colors for output
BLUE='\033[34m'
GREEN='\033[32m'
YELLOW='\033[33m'
RED='\033[31m'
NC='\033[0m' # No Color

print_step() {
    echo -e "${BLUE}🔧 $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_step "Starting FOCAS Monitor build process..."

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the focasmonitor directory."
    exit 1
fi

# Check for required files in parent directory
if [ ! -f "../Fwlib32.dll" ]; then
    print_error "Fwlib32.dll not found in parent directory"
    exit 1
fi

if [ ! -f "../Fwlib32.lib" ]; then
    print_error "Fwlib32.lib not found in parent directory"
    exit 1
fi

if [ ! -f "../fwlib32.h" ]; then
    print_error "fwlib32.h not found in parent directory"
    exit 1
fi

print_success "FANUC library files found"

# Check for MinGW cross-compiler
if ! which i686-w64-mingw32-gcc > /dev/null 2>&1; then
    print_error "MinGW cross-compiler not found. Please install with:"
    echo "  sudo apt install mingw-w64"
    exit 1
fi

print_success "MinGW cross-compiler found"

# Check for CMake
if ! which cmake > /dev/null 2>&1; then
    print_error "CMake not found. Please install CMake."
    exit 1
fi

print_success "CMake found"

# Create build directory
print_step "Creating build directory..."
rm -rf build-windows
mkdir -p build-windows

# Configure with CMake
print_step "Configuring build with CMake..."
cd build-windows
cmake -DCMAKE_TOOLCHAIN_FILE=../../examples/c/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
print_step "Building FOCAS Monitor..."
make

# Go back to main directory
cd ..

# Create release package
print_step "Creating release package..."
rm -rf release
mkdir -p release

# Copy executable
cp build-windows/focasmonitor.exe release/
print_success "Copied focasmonitor.exe"

# Copy FANUC DLLs
cp ../Fwlib32.dll release/
print_success "Copied Fwlib32.dll"

# Copy all FANUC DLLs
cp ../fwlib*.dll release/ 2>/dev/null || print_warning "Some FANUC DLLs may be missing"

# Copy documentation and samples
cp README.md release/
cp machines.txt release/
print_success "Copied documentation and configuration samples"

# Verify the build
print_step "Verifying build..."
if file release/focasmonitor.exe | grep -q "PE32"; then
    print_success "Valid Windows PE32 executable created"
else
    print_error "Invalid executable format"
    exit 1
fi

# Show file sizes
print_step "Build summary:"
echo "Release package contents:"
ls -lh release/

EXE_SIZE=$(stat -c%s release/focasmonitor.exe)
echo "Executable size: $((EXE_SIZE/1024))KB"

print_success "FOCAS Monitor build completed successfully!"
print_step "Release package created in 'release/' directory"
print_step "Ready for Windows deployment"

echo ""
echo "Usage examples:"
echo "  focasmonitor.exe --machines=machines.txt --info=basic"
echo "  focasmonitor.exe --add=CNC1,192.168.1.100,8193 --monitor"
echo "  focasmonitor.exe --help"
