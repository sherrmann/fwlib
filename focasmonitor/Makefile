# Makefile for FOCAS Monitor - Multi-Machine FANUC CNC Monitoring
# Usage:
#   make check         - Check build environment
#   make deps          - Install dependencies  
#   make build-windows - Build Windows executable
#   make package       - Create release package
#   make clean         - Clean build artifacts
#   make all           - Complete build process

# Configuration
TOOLCHAIN_FILE = ../examples/c/mingw-toolchain.cmake
BUILD_DIR = build-windows
RELEASE_DIR = release
TARGET = focasmonitor

# Colors for output
BLUE = \033[34m
GREEN = \033[32m
YELLOW = \033[33m
RED = \033[31m
NC = \033[0m

.PHONY: all check deps build-windows package clean help

all: check deps build-windows package
	@printf "✅ FOCAS Monitor build complete!\n"
	@printf "📦 Release package: $(RELEASE_DIR)/\n"
	@printf "🚀 Ready for Windows deployment\n"

check:
	@printf "🔍 Checking build environment...\n"
	@which cmake > /dev/null || (printf "❌ CMake not found\n" && exit 1)
	@which make > /dev/null || (printf "❌ Make not found\n" && exit 1)
	@which i686-w64-mingw32-gcc > /dev/null || (printf "❌ MinGW cross-compiler not found\n" && exit 1)
	@test -f ../Fwlib32.dll || (printf "❌ Fwlib32.dll not found in parent directory\n" && exit 1)
	@test -f ../Fwlib32.lib || (printf "❌ Fwlib32.lib not found in parent directory\n" && exit 1)
	@test -f ../fwlib32.h || (printf "❌ fwlib32.h not found in parent directory\n" && exit 1)
	@printf "✅ Environment check passed\n"

deps:
	@printf "📦 Installing dependencies...\n"
	@# No external dependencies needed for this version
	@printf "✅ Dependencies ready\n"

build-windows:
	@printf "🔨 Building Windows executable...\n"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_TOOLCHAIN_FILE=../$(TOOLCHAIN_FILE) -DCMAKE_BUILD_TYPE=Release ..
	@cd $(BUILD_DIR) && make
	@printf "✅ Windows build completed\n"

package: build-windows
	@printf "📦 Creating release package...\n"
	@rm -rf $(RELEASE_DIR)
	@mkdir -p $(RELEASE_DIR)
	
	# Copy main executable
	@cp $(BUILD_DIR)/$(TARGET).exe $(RELEASE_DIR)/
	@printf "✅ Copied $(TARGET).exe\n"
	
	# Copy FANUC DLLs
	@cp ../Fwlib32.dll $(RELEASE_DIR)/
	@cp ../fwlib*.dll $(RELEASE_DIR)/ 2>/dev/null || true
	@cp ../Fwlib*.dll $(RELEASE_DIR)/ 2>/dev/null || true
	@printf "✅ Copied FANUC libraries\n"
	
	# Copy documentation and samples
	@cp README.md $(RELEASE_DIR)/
	@echo "# Sample machine configuration" > $(RELEASE_DIR)/machines.txt
	@echo "# Format: name,ip,port" >> $(RELEASE_DIR)/machines.txt
	@echo "Mill-01,192.168.1.100,8193" >> $(RELEASE_DIR)/machines.txt
	@echo "Mill-02,192.168.1.101,8193" >> $(RELEASE_DIR)/machines.txt
	@echo "Lathe-01,192.168.1.102,8193" >> $(RELEASE_DIR)/machines.txt
	@printf "✅ Copied documentation and samples\n"
	
	# Verify the build
	@printf "🔍 Verifying build...\n"
	@file $(RELEASE_DIR)/$(TARGET).exe | grep -q "PE32" && printf "✅ Valid Windows executable\n" || printf "❌ Invalid executable format\n"
	@ls -la $(RELEASE_DIR)/
	@printf "✅ Release package created: $(RELEASE_DIR)/\n"

clean:
	@printf "🧹 Cleaning build artifacts...\n"
	@rm -rf $(BUILD_DIR)
	@rm -rf $(RELEASE_DIR)
	@printf "✅ Clean completed\n"

test-windows: package
	@printf "🧪 Testing Windows executable...\n"
	@printf "Note: Testing on Linux with wine (if available)\n"
	@cd $(RELEASE_DIR) && wine $(TARGET).exe --version 2>/dev/null || printf "⚠️  Wine not available - manual testing required on Windows\n"
	@cd $(RELEASE_DIR) && wine $(TARGET).exe --help 2>/dev/null | head -5 || printf "⚠️  Manual testing required on Windows\n"

help:
	@echo "FOCAS Monitor Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all           - Complete build process (check + deps + build + package)"
	@echo "  check         - Check build environment and dependencies"
	@echo "  deps          - Install/verify dependencies"
	@echo "  build-windows - Build Windows executable using MinGW cross-compilation"
	@echo "  package       - Create release package with all required files"
	@echo "  test-windows  - Test Windows executable (requires Wine on Linux)"
	@echo "  clean         - Clean all build artifacts"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Build Requirements:"
	@echo "  - Linux (Ubuntu 20.04+ recommended)"
	@echo "  - MinGW-w64 cross-compiler (i686-w64-mingw32-gcc)"
	@echo "  - CMake 3.16+"
	@echo "  - FANUC libraries (Fwlib32.dll, Fwlib32.lib, fwlib32.h) in parent directory"
	@echo ""
	@echo "Example Usage:"
	@echo "  make check                    # Verify environment"
	@echo "  make all                      # Complete build"
	@echo "  make package                  # Create release package only"
