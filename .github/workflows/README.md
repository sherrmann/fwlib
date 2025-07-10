# GitHub Actions Workflow

This repository has a **single, comprehensive CI/CD workflow** for building, testing, and distributing the FANUC FOCAS Windows executable.

## 🔄 Workflow Overview

### **CI/CD Pipeline** (`.github/workflows/test-pr.yml`)
**Triggers**: Push, Pull requests, Manual dispatch  
**Purpose**: Complete build, test, and artifact creation pipeline

**What it does**:
- 🏗️ **Linux Build**: Cross-compiles Windows executable using MinGW-w64
- 🪟 **Windows Testing**: Actually runs the executable on Windows Server 2022
- ✅ **Artifact Verification**: Checks executable format, dependencies, error messages
- 📦 **Package Creation**: Creates comprehensive build packages with all DLLs
- ⬆️ **Artifact Upload**: Preserves build outputs for download
- 🚀 **Automatic Releases**: Creates GitHub releases for successful master builds

## 🚀 How to Use

### **Automatic Triggers**
- **Push to any branch**: Workflow runs automatically
- **Pull Requests**: Full build and Windows testing validation
- **Manual**: Use "Run workflow" button in GitHub Actions tab

### **Download Build Artifacts**
1. Go to **Actions** tab in GitHub
2. Click on any completed workflow run
3. Download artifacts from the **Artifacts** section
4. Extract and use the Windows executable package

### **Download Releases (Master Branch)**
1. Go to **Releases** section in GitHub
2. Download the latest release ZIP file
3. Each successful master build creates a new release with timestamp
4. Format: `fanuc-focas-windows-YYYYMMDD-HHMMSS-commit.zip`

## 📦 Build Artifact Contents

Each successful build creates a **complete release package**:
- `fanuc_example.exe` - Windows executable with enhanced error messages
- `liblibconfig.dll` - Configuration library dependency
- **`Fwlib32.dll`** - Main FANUC FOCAS library
- **`Fwlib0i.dll`, `Fwlib0iB.dll`, `Fwlib150.dll`, `Fwlib15i.dll`** - FANUC series libraries
- **`Fwlib160.dll`, `Fwlib16W.dll`, `fwlib30i.dll`** - More FANUC series libraries
- **`Fwlibpm.dll`, `Fwlibpmi.dll`, `fwlibe1.dll`, `fwlibNCG.dll`** - FANUC extension libraries
- **`fwlib0DN.dll`, `fwlib0iD.dll`** - Additional FANUC libraries
- **Complete FANUC FOCAS library set** - All DLLs for maximum compatibility
- `config.cfg` - Sample configuration file (if available)
- `README.md` - Usage instructions (if available)

## 🪟 Windows Testing

The workflow includes **real Windows testing**:
- ✅ **Runs on Windows Server 2022**
- ✅ **Tests executable execution** (not just compilation)
- ✅ **Validates error messages** work correctly
- ✅ **Checks DLL dependencies** are properly included
- ✅ **Verifies package integrity**

## 🛠 Build Environment

**Cross-compilation setup**:
- **Build OS**: Ubuntu Latest
- **Compiler**: MinGW-w64 cross-compiler  
- **Target**: Windows 32-bit (i686)
- **Dependencies**: CMake, Git submodules
- **Build Tools**: Custom build scripts (`build-windows.sh`)

## 📊 Workflow Status

Check workflow status at:
- **Main Workflow**: `https://github.com/USERNAME/fwlib/actions/workflows/test-pr.yml`
- **All Runs**: `https://github.com/USERNAME/fwlib/actions`
- **Latest Releases**: `https://github.com/USERNAME/fwlib/releases`

## 🎯 Key Benefits

✅ **Single Workflow**: One comprehensive pipeline handles everything  
✅ **Cross-Platform**: Linux build environment → Windows executable  
✅ **Real Testing**: Actual execution validation on Windows  
✅ **Simple Triggers**: Push, PR, manual - no tags or schedules  
✅ **Easy Downloads**: Build artifacts available from every run  
✅ **Automatic Releases**: GitHub releases created for master builds  
✅ **Complete Packages**: All DLLs and dependencies included  

## 🔧 Customization

To modify the build process, edit:
- `examples/c/build-windows.sh` - Main build script
- `examples/c/CMakeLists.txt` - CMake configuration  
- `.github/workflows/test-pr.yml` - Workflow definition

## 🔍 Troubleshooting

### Build Failures
1. Check the **Actions** tab for detailed logs
2. Verify **git submodules** are properly initialized
3. Ensure **build scripts** have execute permissions
4. Check **MinGW-w64** cross-compiler availability

### Windows Testing Failures
1. Look for **DLL dependency** issues in logs
2. Verify **executable format** is correct (PE32)
3. Check **error message** output matches expected format

### Artifact Issues
1. Ensure **workflow completed** successfully
2. Check **artifact upload** step in workflow logs
3. Verify **artifact retention** settings (30 days)

---

This single workflow provides everything needed for reliable FANUC FOCAS Windows executable builds and testing.
