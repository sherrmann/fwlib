# FANUC FOCAS Library
[![CI/CD Pipeline](https://github.com/strangesast/fwlib/actions/workflows/focasmonitor-ci.yml/badge.svg)](https://github.com/strangesast/fwlib/actions/workflows/focasmonitor-ci.yml)

Comprehensive FANUC FOCAS library with examples and production-ready multi-machine monitoring tools.

## 🚀 Quick Start

### Production Multi-Machine Monitoring
For production environments, use **FOCAS Monitor**:
```bash
# Download latest release from GitHub Releases
# Extract focasmonitor package
cd focasmonitor
focasmonitor.exe --machines=machines.txt --monitor --interval=30
```

### Legacy Single-Machine Examples  
For development and testing:
```bash
# Extract fanuc-examples package
cd fanuc-examples  
fanuc_example.exe --ip=192.168.1.100 --port=8193 --info=all
```

## 📦 Release Packages

Each CI/CD run produces three downloadable artifacts:

- **`focasmonitor-{sha}`** - Production multi-machine monitoring application
- **`fanuc-examples-{sha}`** - Legacy single-machine example application  
- **`fanuc-focas-complete-{sha}`** - Complete package with both applications

Additionally, the workflow creates timestamped GitHub releases on successful builds:

- **`focasmonitor-build-YYYYMMDD-HHMMSS-{commit}.zip`** - Versioned production package
- **`fanuc-examples-build-YYYYMMDD-HHMMSS-{commit}.zip`** - Versioned examples package
- **`fanuc-focas-complete-build-YYYYMMDD-HHMMSS-{commit}.zip`** - Complete versioned package

All packages include the complete set of 14 FANUC FOCAS library DLLs supporting all machine series.

## 🏗️ Development

### CI/CD Pipeline
- **Single Workflow**: One GitHub Actions workflow handles both examples and production code
- **Cross-Platform Testing**: Build on Linux, test on Windows
- **Automatic Releases**: Every push/PR creates timestamped release packages
- **Complete DLL Coverage**: All 14 FANUC library DLLs included in every package

### Building from Source
```bash
# Build examples
cd examples/c
make all

# Build FOCAS Monitor  
cd focasmonitor
make all
```

## 📁 Repository Structure

- **`/focasmonitor/`** - Production multi-machine monitoring application
- **`/examples/`** - Legacy examples and development tools
- **`/.github/workflows/`** - Unified CI/CD pipeline
- **`/*.dll`** - Complete FANUC FOCAS library collection (14 DLLs)

## 🔧 System Requirements

- **Windows 7 or later** (32-bit or 64-bit)  
- **Network connectivity** to FANUC CNC machines
- **FANUC machines** with FOCAS support enabled

## 📚 Documentation

- [`/focasmonitor/README.md`](focasmonitor/README.md) - Production application documentation
- [`/focasmonitor/DEPLOYMENT.md`](focasmonitor/DEPLOYMENT.md) - Deployment guide
- [`/examples/c/README.md`](examples/c/README.md) - Legacy examples documentation

## 🐳 Docker Support

Build the base image:
```bash
docker build .
```

Build example containers:
```bash
docker build examples/c/Dockerfile
```

## 🔗 Linux Development

Link appropriate `libfwlib*.so` (based on platform) to `libfwlib32.so.1` and `libfwlib32.so`:

```bash
# On Linux x86_64:
ln -s libfwlib32-linux-x64.so.1.0.5 libfwlib32.so
```

See individual example folders for detailed platform-specific instructions.
