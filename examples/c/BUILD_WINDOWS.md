# Building Windows Executable for FANUC FOCAS Library

This document provides step-by-step instructions for building a Windows executable from the FANUC FOCAS library example using cross-compilation on Linux.

## Quick Start

The fastest way to build is using the provided build script:

```bash
cd /workspaces/fwlib/examples/c
./build-windows.sh
```

## Manual Build Process

### Prerequisites

1. **Install MinGW-w64 cross-compiler:**
   ```bash
   sudo apt update
   sudo apt install -y mingw-w64 cmake
   ```

2. **Verify installation:**
   ```bash
   i686-w64-mingw32-gcc --version
   x86_64-w64-mingw32-gcc --version
   ```

### Step-by-Step Build

#### 1. Initialize Dependencies
```bash
cd /workspaces/fwlib/examples/c
git submodule update --init --recursive
```

#### 2. Build libconfig for Windows (32-bit)
```bash
cd extern/libconfig
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../../../mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
cd ../../..
```

#### 3. Build Main Project
```bash
mkdir build-windows && cd build-windows
cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ..
make fanuc_example
```

#### 4. Create Release Package
```bash
mkdir windows-release
cp build-windows/bin/fanuc_example.exe windows-release/
cp build-windows/bin/liblibconfig.dll windows-release/
cp ../../Fwlib32.dll windows-release/
```

## Configuration Files

### mingw-toolchain.cmake
Cross-compilation toolchain for 32-bit Windows:
```cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

### Sample config.cfg
```
# FANUC CNC Configuration
ip = "192.168.1.100";
port = 8193;
```

## Key Modifications Made

### 1. Fixed Microsoft-specific Functions
**Problem:** Original code used `_dupenv_s()` which is MSVC-specific.

**Solution:** Replaced with standard `getenv()`:
```c
// Before (MSVC-specific):
if (_dupenv_s(&pTmp, &n, "DEVICE_IP") == 0 && pTmp != NULL) {
    snprintf(conf->ip, 100, "%s", pTmp);
}

// After (Standard C):
pTmp = getenv("DEVICE_IP");
if (pTmp != NULL) {
    snprintf(conf->ip, 100, "%s", pTmp);
}
```

### 2. Architecture Compatibility
**Problem:** FANUC DLLs are 32-bit, initial attempt was 64-bit.

**Solution:** Changed toolchain to target i686 (32-bit) instead of x86_64.

### 3. Library Linking
**Problem:** CMake wasn't properly linking the FANUC library.

**Solution:** Added explicit library linking in CMakeLists.txt:
```cmake
if (WIN32)
  target_link_libraries(fanuc_example "${CMAKE_SOURCE_DIR}/../../Fwlib32.lib")
endif()
```

## Output Files

The build process creates:
- `fanuc_example.exe` - Main executable (32-bit Windows)
- `liblibconfig.dll` - Configuration library
- `Fwlib32.dll` - FANUC FOCAS library
- `config.cfg` - Sample configuration
- `README.md` - Usage instructions

## Usage on Windows

### Command Line Arguments
```cmd
fanuc_example.exe --ip=192.168.1.100 --port=8193
```

### Configuration File
```cmd
fanuc_example.exe --config=config.cfg
```

### Environment Variables
```cmd
set DEVICE_IP=192.168.1.100
set DEVICE_PORT=8193
fanuc_example.exe
```

## Troubleshooting

### Common Issues

1. **MinGW not found:**
   ```bash
   sudo apt install mingw-w64
   ```

2. **Git submodules not initialized:**
   ```bash
   git submodule update --init --recursive
   ```

3. **Wrong architecture (64-bit vs 32-bit):**
   - FANUC DLLs are 32-bit
   - Use `i686-w64-mingw32-gcc` not `x86_64-w64-mingw32-gcc`

4. **Library not found errors:**
   - Ensure `Fwlib32.lib` exists in repository root
   - Check that libconfig was built successfully

### Verification Commands

```bash
# Check executable format
file windows-release/fanuc_example.exe

# Check DLL dependencies
i686-w64-mingw32-objdump -p windows-release/fanuc_example.exe | grep "DLL Name"

# Check if all files are present
ls -la windows-release/
```

## Build Environment

Tested on:
- Ubuntu 24.04 (Noble)
- MinGW-w64 13.2.0
- CMake 3.28.3
- Target: Windows 32-bit (i686)

## Notes

- The build creates a 32-bit Windows executable compatible with Windows 7 and later
- All required DLLs are included in the release package
- The executable does not require Visual Studio runtime (uses msvcrt.dll)
- Cross-compilation is performed entirely on Linux without needing Windows
