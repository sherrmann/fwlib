# GitHub Actions Workflows

This repository includes automated CI/CD workflows for building, testing, and releasing the FANUC FOCAS Windows executable.

# GitHub Actions Workflows

This repository includes automated CI/CD workflows for building, testing, and releasing the FANUC FOCAS Windows executable.

## 🔄 Workflows Overview

### 1. **CI/CD Pipeline** (`.github/workflows/test-pr.yml`)
**Trigger**: Push to any branch, Pull requests, Manual dispatch  
**Purpose**: Main CI/CD pipeline with cross-platform build and Windows testing

**What it does**:
- 🏗️ **Linux Build**: Cross-compiles Windows executable using MinGW-w64
- 🪟 **Windows Testing**: Actually runs the executable on Windows Server 2022
- ✅ **Artifact Verification**: Checks executable format, dependencies, error messages
- 📦 **Package Creation**: Creates comprehensive build packages
- ⬆️ **Artifact Upload**: Preserves build outputs for download

### 2. **CI** (`.github/workflows/ci.yml`)  
**Trigger**: Push to `master`/`main` (C code changes only)  
**Purpose**: Lightweight continuous integration for code changes

**What it does**:
- ✅ Quick build verification
- ✅ Basic artifact validation
- ✅ Fast feedback for code changes

### 3. **Build and Release** (`.github/workflows/build-and-release.yml`)
**Trigger**: Push, Pull requests, Manual dispatch  
**Purpose**: Comprehensive build, test, and artifact pipeline

**What it does**:
- 🔨 Cross-platform build verification (Linux → Windows)
- 🧪 Package content validation and testing
- 📦 Comprehensive artifact creation with all FANUC DLLs
- ⬆️ **Artifact Upload** for download and distribution
- 📝 Build verification and validation

### 4. **Quality Assurance** (`.github/workflows/quality.yml`)
**Trigger**: Push, Pull requests, Manual dispatch  
**Purpose**: Code quality, documentation, and dependency validation

**What it does**:
- 📚 **Documentation validation** (README, markdown files)
- 🔍 **Code quality checks** (C code validation)
- 🔗 **Dependency verification** (git submodules, external deps)
- 🧪 **Build system testing** (scripts, Makefiles, CMake)
- ✅ **Continuous quality assurance** for all changes

### 5. **CMake** (`.github/workflows/cmake.yml`) 
**Trigger**: Push, Pull requests, Manual dispatch  
**Purpose**: CMake build system validation

**What it does**:
- 🔨 CMake configuration testing
- ✅ Build system verification
- 🧪 Cross-platform compatibility checks

### 6. **Windows CMake** (`.github/workflows/windows-cmake.yml`)
**Trigger**: Push, Pull requests, Manual dispatch  
**Purpose**: Native Windows CMake builds

**What it does**:
- 🔨 Native Windows build verification
- 🧪 Windows-specific testing
- 📦 Windows artifact validation
- 🎯 Platform-specific compatibility checks

## 🚀 How to Use

### Testing Any Changes (Automatic)
- **Any Push**: The main CI/CD Pipeline runs automatically
- **Pull Requests**: Full build and Windows testing validation
- **Manual Testing**: Use workflow_dispatch for on-demand runs

### Creating Build Artifacts
1. **Commit your changes** to any branch
2. **Push to trigger builds**:
   ```bash
   git push origin feature-branch
   ```
3. **GitHub Actions automatically**:
   - Runs comprehensive build and test pipeline
   - Creates build artifacts with all FANUC DLLs
   - Validates Windows executable functionality
   - Uploads artifacts for download

### Manual Workflow Trigger
- Navigate to **Actions** tab in GitHub
- Select any workflow (e.g., **CI/CD Pipeline**)
- Click **"Run workflow"** button
- Choose branch and run

## 🪟 Windows Testing

The **CI/CD Pipeline** includes actual Windows testing:
- ✅ **Runs on Windows Server 2022**
- ✅ **Tests executable execution** (not just compilation)
- ✅ **Validates error messages** work correctly
- ✅ **Checks DLL dependencies** are properly included
- ✅ **Verifies package integrity**

## 🎯 Key Features

✅ **Cross-Platform Build**: Linux → Windows cross-compilation  
✅ **Real Windows Testing**: Actual execution on Windows runners  
✅ **Comprehensive Validation**: Code, docs, dependencies, build system  
✅ **Automated Builds**: Push and PR triggered build creation  
✅ **Quality Assurance**: Continuous dependency and quality checks  
✅ **Multiple Triggers**: Push, PR, manual dispatch  
✅ **Artifact Management**: Build outputs preserved and downloadable  

## 📦 Build Artifact Contents

Automated builds include:
- `fanuc_example.exe` - Windows executable with enhanced error messages
- **All FANUC FOCAS DLLs** - Complete library set for compatibility
- `liblibconfig.dll` - Configuration library
- `config.cfg` - Sample configuration file
- `README.md` - Usage instructions and troubleshooting

## 🛠 Build Environment

**Workflows use**:
- **OS**: Ubuntu Latest
- **Compiler**: MinGW-w64 cross-compiler
- **Target**: Windows 32-bit (i686)
- **Dependencies**: CMake, Git submodules
- **Build Tools**: Custom build scripts (`build-windows.sh`)

## 🔧 Workflow Files Summary

| Workflow | File | Triggers | Purpose |
|----------|------|----------|---------|
| **CI/CD Pipeline** | `test-pr.yml` | Push, PR, Manual | Main build & Windows testing |
| **Build and Release** | `build-and-release.yml` | Push, PR, Manual | Comprehensive pipeline + artifacts |
| **Quality Assurance** | `quality.yml` | Push, PR, Manual | Code quality & docs validation |
| **CI** | `ci.yml` | Push, PR, Manual | Quick build verification |
| **CMake** | `cmake.yml` | Push, PR, Manual | Build system validation |
| **Windows CMake** | `windows-cmake.yml` | Push, PR, Manual | Native Windows builds |

## 🔧 Customization

### Build Configuration
Edit these files to customize builds:
- `examples/c/build-windows.sh` - Main build script
- `examples/c/CMakeLists.txt` - CMake configuration
- `.github/workflows/*.yml` - Workflow definitions

## 🎯 Benefits

✅ **Automated Quality**: Every push is built and tested on Windows  
✅ **Cross-Platform Build**: Linux build environment → Windows executable  
✅ **Real Windows Testing**: Actual execution validation, not just compilation  
✅ **Consistent Builds**: Reproducible, automated build process  
✅ **Easy Distribution**: Downloadable packages with all DLLs from any workflow run  
✅ **Documentation**: Auto-generated build logs and validation reports  
✅ **Artifact Management**: Build outputs preserved and accessible  
✅ **Quality Assurance**: On-demand quality checks for code and dependencies  
✅ **Documentation Validation**: Ensures docs stay up-to-date  
✅ **Multiple Environments**: Ubuntu + Windows Server testing  
✅ **Comprehensive Coverage**: Build, test, package, validate quality  

## 🔍 Troubleshooting

### Build Failures
1. Check the **Actions** tab for detailed logs
2. **CI/CD Pipeline** provides the most comprehensive output
3. Verify **submodules** are properly initialized
4. Ensure **build scripts** have execute permissions
5. Check **MinGW-w64** cross-compiler availability

### Windows Testing Failures  
1. Check **Windows job** in CI/CD Pipeline workflow
2. Look for **DLL dependency** issues in logs
3. Verify **executable format** is correct (PE32)
4. Check **error message** output matches expected format

### Artifact Download Issues
1. Check **workflow run** completion in Actions tab
2. Verify **build succeeded** without errors
3. Look for **artifact** section in completed workflow runs
4. Ensure **workflow** has upload permissions
5. Check **workflow file** artifact upload configuration

### Quality Assurance Failures
1. Check **documentation** syntax and links
2. Verify **C code** compiles without warnings
3. Ensure **git submodules** are properly configured
4. Check **dependency** availability and versions

---

These workflows provide a complete CI/CD pipeline for the FANUC FOCAS project, ensuring reliable cross-platform builds, real Windows testing, and automated distribution of Windows executables through build artifacts.
