#!/bin/bash
# Quick verification script to check if the Windows build environment is ready

echo "🔍 Checking Windows Build Environment"
echo "====================================="

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

check_passed=0
check_failed=0

check_command() {
    local cmd="$1"
    local desc="$2"
    
    if command -v "$cmd" &> /dev/null; then
        echo -e "${GREEN}✅ $desc: $(which $cmd)${NC}"
        ((check_passed++))
        if [ "$cmd" == "i686-w64-mingw32-gcc" ]; then
            echo "   Version: $(i686-w64-mingw32-gcc --version | head -1)"
        fi
    else
        echo -e "${RED}❌ $desc: not found${NC}"
        ((check_failed++))
    fi
}

check_file() {
    local file="$1"
    local desc="$2"
    
    if [ -f "$file" ]; then
        echo -e "${GREEN}✅ $desc: $file${NC}"
        ((check_passed++))
    else
        echo -e "${RED}❌ $desc: $file not found${NC}"
        ((check_failed++))
    fi
}

check_directory() {
    local dir="$1"
    local desc="$2"
    
    if [ -d "$dir" ]; then
        echo -e "${GREEN}✅ $desc: $dir${NC}"
        ((check_passed++))
    else
        echo -e "${YELLOW}⚠️  $desc: $dir not found (will be created)${NC}"
    fi
}

# Check required commands
echo "📋 Checking required tools..."
check_command "cmake" "CMake"
check_command "make" "Make"
check_command "git" "Git"
check_command "zip" "Zip (optional)"
check_command "i686-w64-mingw32-gcc" "MinGW 32-bit C compiler"
check_command "i686-w64-mingw32-g++" "MinGW 32-bit C++ compiler"
check_command "i686-w64-mingw32-objdump" "MinGW objdump"

echo ""
echo "📁 Checking project files..."
check_file "CMakeLists.txt" "Main CMakeLists.txt"
check_file "src/main.c" "Main source file"
check_file "mingw-toolchain.cmake" "MinGW toolchain file"
check_file "build-windows.sh" "Build script"
check_file "../../Fwlib32.dll" "FANUC DLL"
check_file "../../Fwlib32.lib" "FANUC import library"
check_file "../../fwlib32.h" "FANUC header"

echo ""
echo "📂 Checking project structure..."
check_directory "src" "Source directory"
check_directory "extern" "External dependencies directory"
check_directory "extern/libconfig" "libconfig directory"

echo ""
echo "🔗 Checking git submodules..."
if [ -f ".gitmodules" ]; then
    while IFS= read -r line; do
        if [[ $line =~ path[[:space:]]*=[[:space:]]*(.+) ]]; then
            submodule_path="${BASH_REMATCH[1]}"
            check_directory "$submodule_path" "Submodule $submodule_path"
        fi
    done < .gitmodules
else
    echo -e "${YELLOW}⚠️  .gitmodules not found${NC}"
fi

echo ""
echo "📊 Summary:"
echo "=========="
echo -e "Passed: ${GREEN}$check_passed${NC}"
echo -e "Failed: ${RED}$check_failed${NC}"

if [ $check_failed -eq 0 ]; then
    echo -e "${GREEN}🎉 Environment is ready for Windows build!${NC}"
    echo ""
    echo "Run the build:"
    echo "  ./build-windows.sh"
    exit 0
else
    echo -e "${RED}❌ Environment needs setup${NC}"
    echo ""
    echo "Install missing packages:"
    echo "  sudo apt update && sudo apt install -y mingw-w64 cmake"
    echo ""
    echo "Initialize submodules:"
    echo "  git submodule update --init --recursive"
    exit 1
fi
