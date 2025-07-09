# Windows Build System for FANUC FOCAS Library

## 📁 Files Created for Windows Building

### 🔧 Build Scripts
| File | Purpose | Usage |
|------|---------|-------|
| `build-windows.sh` | Automated build script | `./build-windows.sh` |
| `check-environment.sh` | Environment verification | `./check-environment.sh` |
| `Makefile` | Make-based build system | `make all` |

### ⚙️ Configuration Files
| File | Purpose | Description |
|------|---------|-------------|
| `mingw-toolchain.cmake` | CMake toolchain | Cross-compilation setup for 32-bit Windows |
| `build-config.env` | Build configuration | All build parameters and settings |
| `BUILD_WINDOWS.md` | Documentation | Complete build instructions |

### 📋 Quick Reference

#### One-Command Build
```bash
# Easiest - automated script
./build-windows.sh

# Or using Make
make all

# Or just check environment first
./check-environment.sh
```

#### Manual Step-by-Step
```bash
# 1. Check prerequisites
./check-environment.sh

# 2. Install dependencies (if needed)
sudo apt install -y mingw-w64 cmake

# 3. Initialize submodules
git submodule update --init --recursive

# 4. Build
make build

# 5. Create package
make package
```

#### Build Targets (Makefile)
```bash
make check      # Check environment
make deps       # Install dependencies  
make build      # Build executable
make package    # Create release
make verify     # Verify build
make zip        # Create ZIP package
make clean      # Clean all
make help       # Show help
```

## 🎯 What Gets Built

### Output Files
- `fanuc_example.exe` - 32-bit Windows executable (~245KB)
- `liblibconfig.dll` - Configuration library (~268KB)  
- `Fwlib32.dll` - FANUC FOCAS library (~545KB)
- `config.cfg` - Sample configuration file
- `README.md` - Usage instructions

### Package Structure
```
windows-release/
├── fanuc_example.exe      # Main executable
├── liblibconfig.dll       # Config library
├── Fwlib32.dll           # FANUC library
├── config.cfg            # Sample config
└── README.md             # Usage guide
```

## 🔧 Technical Details

### Cross-Compilation Setup
- **Target**: 32-bit Windows (i686)
- **Compiler**: MinGW-w64 (i686-w64-mingw32-gcc)
- **Dependencies**: Built from source for Windows
- **Architecture**: PE32 executable for Intel 80386

### Key Modifications Made
1. **Fixed MSVC-specific functions** (`_dupenv_s` → `getenv`)
2. **32-bit architecture** (FANUC DLLs are 32-bit only)
3. **Explicit library linking** (CMakeLists.txt updates)
4. **Cross-compiled dependencies** (libconfig for Windows)

### Build Environment
- **OS**: Linux (Ubuntu 24.04+)
- **Tools**: MinGW-w64, CMake 3.28+, Make
- **Output**: Windows 7+ compatible executable

## 🚀 Usage Examples

### On Windows Machine
```cmd
# Command line
fanuc_example.exe --ip=192.168.1.100 --port=8193

# Config file  
fanuc_example.exe --config=config.cfg

# Environment variables
set DEVICE_IP=192.168.1.100
set DEVICE_PORT=8193
fanuc_example.exe
```

## 🔍 Verification Commands

```bash
# Check file type
file windows-release/fanuc_example.exe

# Check dependencies
i686-w64-mingw32-objdump -p windows-release/fanuc_example.exe | grep "DLL Name"

# Expected output:
# PE32 executable (console) Intel 80386, for MS Windows
# DLL Name: KERNEL32.dll
# DLL Name: msvcrt.dll  
# DLL Name: Fwlib32.dll
# DLL Name: liblibconfig.dll
```

## 🎉 Success Indicators

✅ **Environment Ready**: `./check-environment.sh` shows all green  
✅ **Build Success**: `fanuc_example.exe` created in `windows-release/`  
✅ **Correct Format**: File type shows "PE32 executable ... Intel 80386"  
✅ **Dependencies**: All required DLLs present and linked  
✅ **Package Complete**: ZIP file created with all components  

## 🔄 Rebuild Process

For future builds or modifications:

1. **Quick rebuild** (code changes only):
   ```bash
   make quick-build
   ```

2. **Clean rebuild** (dependency changes):
   ```bash
   make clean && make all
   ```

3. **Full rebuild** (from scratch):
   ```bash
   ./build-windows.sh
   ```

## 📁 Git Configuration

### .gitignore Files
The project includes comprehensive `.gitignore` files to prevent build artifacts from being committed:

- **Root `.gitignore`** (`/workspaces/fwlib/.gitignore`) - Project-wide exclusions
- **C Examples `.gitignore`** (`examples/c/.gitignore`) - C-specific build artifacts

### What's Excluded
- All build directories (`build-windows/`, `windows-release/`, etc.)
- Compiled binaries (`.exe`, `.dll`, `.lib`, `.obj`)  
- CMake generated files (`CMakeCache.txt`, `CMakeFiles/`)
- IDE configuration files (`.vscode/`, `.idea/`)
- Dependency builds (`extern/*/build/`)
- Log files (`focas.log`, `*.log`)
- Package files (`*.zip`)
- Temporary and backup files

### Clean Repository
```bash
# Check what would be ignored
git status --ignored

# Clean all ignored files (be careful!)
git clean -fdX

# Clean only build artifacts
rm -rf build-windows/ windows-release/ extern/*/build/
```

## 📞 Troubleshooting

| Issue | Solution |
|-------|----------|
| MinGW not found | `sudo apt install mingw-w64` |
| Submodules missing | `git submodule update --init --recursive` |
| Wrong architecture | Use `i686-w64-mingw32-gcc` (32-bit) |
| Library errors | Ensure `Fwlib32.lib` exists in repo root |
| Build fails | Run `./check-environment.sh` first |

This build system provides multiple ways to build the Windows executable, from one-command automation to detailed manual control, with comprehensive verification and documentation.
