#!/bin/bash
# Build script for Windows executable using MinGW cross-compilation
# This script builds the FANUC FOCAS library example for Windows from Linux

set -e  # Exit on any error

echo "🔨 Building FANUC Windows Example with MinGW"
echo "============================================"

# Configuration
PROJECT_ROOT="/workspaces/fwlib/examples/c"
BUILD_DIR="build-windows"
RELEASE_DIR="windows-release"
TOOLCHAIN_FILE="mingw-toolchain.cmake"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_step() {
    echo -e "${BLUE}📌 $1${NC}"
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

# Check prerequisites
print_step "Checking prerequisites..."

# Check if MinGW is installed
if ! command -v i686-w64-mingw32-gcc &> /dev/null; then
    print_error "MinGW-w64 not found. Installing..."
    sudo apt update && sudo apt install -y mingw-w64 cmake
    print_success "MinGW-w64 installed"
else
    print_success "MinGW-w64 found"
fi

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ] || [ ! -f "src/main.c" ]; then
    print_error "Please run this script from the examples/c directory"
    exit 1
fi

# Navigate to project root
cd "$PROJECT_ROOT"
print_success "Working directory: $(pwd)"

# Step 1: Initialize git submodules
print_step "Initializing git submodules..."
if [ ! -f "extern/libconfig/CMakeLists.txt" ]; then
    git submodule update --init --recursive
    print_success "Git submodules initialized"
else
    print_success "Git submodules already initialized"
fi

# Step 2: Build libconfig dependency for Windows
print_step "Building libconfig dependency for Windows..."
cd extern/libconfig
if [ -d "build" ]; then
    rm -rf build
    print_warning "Cleaned existing libconfig build directory"
fi

mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../../../${TOOLCHAIN_FILE} -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
print_success "libconfig built successfully"

# Step 3: Build main project
cd "$PROJECT_ROOT"
print_step "Building main FANUC example..."

if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
    print_warning "Cleaned existing build directory"
fi

mkdir "$BUILD_DIR" && cd "$BUILD_DIR"
cmake -DCMAKE_TOOLCHAIN_FILE="../${TOOLCHAIN_FILE}" -DCMAKE_BUILD_TYPE=Release ..

# Build only the main target (skip tests that would fail on Linux)
make fanuc_example
print_success "FANUC example built successfully"

# Step 4: Create release package
cd "$PROJECT_ROOT"
print_step "Creating Windows release package..."

if [ -d "$RELEASE_DIR" ]; then
    rm -rf "$RELEASE_DIR"
    print_warning "Cleaned existing release directory"
fi

mkdir "$RELEASE_DIR"

# Copy executable and dependencies
cp "$BUILD_DIR/bin/fanuc_example.exe" "$RELEASE_DIR/"
cp "$BUILD_DIR/bin/liblibconfig.dll" "$RELEASE_DIR/"
cp "../../Fwlib32.dll" "$RELEASE_DIR/"

# Copy configuration files if they exist
if [ -f "$RELEASE_DIR/../config.cfg" ]; then
    cp "$RELEASE_DIR/../config.cfg" "$RELEASE_DIR/"
fi
if [ -f "$RELEASE_DIR/../README.md" ]; then
    cp "$RELEASE_DIR/../README.md" "$RELEASE_DIR/"
fi

print_success "Release package created in $RELEASE_DIR/"

# Step 5: Verify the build
print_step "Verifying the build..."
if [ -f "$RELEASE_DIR/fanuc_example.exe" ]; then
    EXE_SIZE=$(stat -c%s "$RELEASE_DIR/fanuc_example.exe")
    EXE_TYPE=$(file "$RELEASE_DIR/fanuc_example.exe")
    
    print_success "Executable created: fanuc_example.exe (${EXE_SIZE} bytes)"
    print_success "File type: ${EXE_TYPE}"
    
    # Check dependencies
    print_step "Checking DLL dependencies..."
    i686-w64-mingw32-objdump -p "$RELEASE_DIR/fanuc_example.exe" | grep "DLL Name" || true
    
    # List release contents
    print_step "Release package contents:"
    ls -la "$RELEASE_DIR/"
    
    # Create ZIP package
    if command -v zip &> /dev/null; then
        ZIP_NAME="fanuc-windows-example-$(date +%Y%m%d-%H%M%S).zip"
        zip -r "$ZIP_NAME" "$RELEASE_DIR/"
        print_success "ZIP package created: $ZIP_NAME"
    fi
    
else
    print_error "Build failed - executable not found"
    exit 1
fi

echo ""
echo "🎉 Build completed successfully!"
echo "📦 Windows executable ready at: $PROJECT_ROOT/$RELEASE_DIR/fanuc_example.exe"
echo ""
echo "To use on Windows:"
echo "  fanuc_example.exe --ip=192.168.1.100 --port=8193"
echo "  fanuc_example.exe --config=config.cfg"
echo ""
