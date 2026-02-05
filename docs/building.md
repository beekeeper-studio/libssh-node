# Building libssh-node

This guide explains how to build libssh-node from source on different platforms.

## Prerequisites

### All Platforms

- Node.js 16 or later
- Python 3.7 or later (for node-gyp)
- yarn or npm

### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  libssh-dev \
  python3 \
  git
```

### Linux (Fedora/RHEL/CentOS)

```bash
sudo dnf install -y \
  gcc-c++ \
  make \
  libssh-devel \
  python3 \
  git
```

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install libssh
brew install libssh
```

### Windows

1. Install Visual Studio 2019 or later with C++ build tools
2. Install Python 3.7+
3. Install vcpkg:

```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install libssh:x64-windows
```

## Building

### Standard Build

```bash
# Install dependencies
yarn install

# Build native module
yarn build:native

# Build TypeScript
yarn build:ts

# Or build everything
yarn build
```

### Development Build

For development with debug symbols:

```bash
# Build with debug info
yarn build:native --debug

# Or using node-gyp directly
node-gyp rebuild --debug
```

### Clean Build

```bash
# Clean previous builds
yarn clean

# Rebuild everything
yarn rebuild
```

## Building for Electron

### Basic Electron Build

```bash
# Install electron-rebuild
yarn add --dev electron-rebuild

# Rebuild for your Electron version
yarn electron-rebuild -v 31.0.0
```

### Manual Electron Build

```bash
# Set Electron headers
export npm_config_target=31.0.0
export npm_config_arch=x64
export npm_config_target_arch=x64
export npm_config_disturl=https://electronjs.org/headers
export npm_config_runtime=electron
export npm_config_build_from_source=true

# Build
yarn rebuild
```

### Build Script for Electron

Add to package.json:

```json
{
  "scripts": {
    "rebuild:electron": "electron-rebuild -v 31.0.0",
    "rebuild:electron:debug": "electron-rebuild -v 31.0.0 --debug"
  }
}
```

## Platform-Specific Build Instructions

### Linux

#### ARM64 (Raspberry Pi, ARM servers)

```bash
# Install ARM64 dependencies
sudo apt-get install -y libssh-dev:arm64

# Build
yarn build
```

#### Cross-Compilation

```bash
# Install cross-compilation tools
sudo apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# Set environment
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export npm_config_arch=arm64

# Build
yarn build:native
```

### macOS

#### Universal Binary (x64 + ARM64)

```bash
# Install both architectures of libssh
arch -x86_64 brew install libssh
arch -arm64 brew install libssh

# Build universal binary
node-gyp rebuild --arch=universal
```

#### ARM64 (Apple Silicon)

```bash
# Should work out of the box
yarn build
```

#### x64 (Intel)

```bash
# Force x64 build on Apple Silicon
arch -x86_64 yarn build
```

### Windows

#### x64 Build

```bash
# Ensure vcpkg is configured
vcpkg install libssh:x64-windows

# Build
yarn build
```

#### Debug Build

```bash
# Build with debugging
yarn build:native --debug

# Or specify config
node-gyp rebuild --debug --configuration=Debug
```

## Troubleshooting

### Common Build Errors

#### "libssh.h: No such file or directory"

**Solution**: Install libssh development headers

```bash
# Linux
sudo apt-get install libssh-dev

# macOS
brew install libssh

# Windows
vcpkg install libssh
```

#### "Python not found"

**Solution**: Install Python 3.7+

```bash
# Linux
sudo apt-get install python3

# macOS
brew install python3

# Windows
# Download from python.org
```

#### "MSBuild.exe not found" (Windows)

**Solution**: Install Visual Studio Build Tools

1. Download Visual Studio Installer
2. Install "Desktop development with C++"
3. Restart terminal

#### "node-gyp rebuild failed"

**Solution**: Clear cache and rebuild

```bash
# Clear node-gyp cache
rm -rf ~/.node-gyp

# Clear build artifacts
yarn clean

# Rebuild
yarn rebuild
```

### libssh Version Issues

#### Check libssh Version

```bash
# Linux
pkg-config --modversion libssh

# macOS
brew info libssh
```

#### Minimum Version Required

libssh-node requires libssh 0.9.0 or later.

**Update if needed**:

```bash
# Linux
sudo apt-get update
sudo apt-get upgrade libssh-dev

# macOS
brew upgrade libssh
```

### Linking Errors

#### Linux: Library Not Found

```bash
# Update library cache
sudo ldconfig

# Check library path
pkg-config --libs libssh
```

#### macOS: Library Not Found

```bash
# Check Homebrew library path
brew --prefix libssh

# Add to environment
export LDFLAGS="-L$(brew --prefix libssh)/lib"
export CPPFLAGS="-I$(brew --prefix libssh)/include"

# Rebuild
yarn rebuild
```

#### Windows: Library Not Found

Ensure vcpkg is integrated:

```powershell
vcpkg integrate install
```

## Build Configuration

### Custom binding.gyp

Edit `binding.gyp` to customize build:

```python
{
  'targets': [{
    'target_name': 'libssh_node',
    'defines': [
      'NAPI_VERSION=8',
      # Add custom defines
    ],
    'include_dirs': [
      # Add custom include paths
    ],
    'libraries': [
      # Add custom libraries
    ]
  }]
}
```

### Environment Variables

Control the build with environment variables:

```bash
# Target architecture
export npm_config_arch=x64

# Debug build
export npm_config_debug=true

# Custom include/lib paths
export CPPFLAGS="-I/custom/include"
export LDFLAGS="-L/custom/lib"

# Rebuild
yarn rebuild
```

## Continuous Integration

### GitHub Actions

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]

    runs-on: ${{ matrix.os }}

    steps:
      - uses: actions/checkout@v3

      - uses: actions/setup-node@v3
        with:
          node-version: '18'

      - name: Install libssh (Ubuntu)
        if: runner.os == 'Linux'
        run: sudo apt-get install -y libssh-dev

      - name: Install libssh (macOS)
        if: runner.os == 'macOS'
        run: brew install libssh

      - name: Install libssh (Windows)
        if: runner.os == 'Windows'
        run: |
          vcpkg install libssh:x64-windows
          vcpkg integrate install

      - name: Install dependencies
        run: yarn install

      - name: Build
        run: yarn build

      - name: Test
        run: yarn test
```

## Distribution

### Prebuilt Binaries

To distribute prebuilt binaries:

1. Install node-pre-gyp:

```bash
yarn add node-pre-gyp
```

2. Configure package.json:

```json
{
  "binary": {
    "module_name": "libssh_node",
    "module_path": "./build/Release/",
    "host": "https://github.com/your-org/libssh-node/releases/download/"
  }
}
```

3. Build and publish:

```bash
# Build for current platform
node-pre-gyp rebuild

# Package
node-pre-gyp package

# Publish to GitHub releases
node-pre-gyp publish
```

## Development Tips

### Watch Mode

For TypeScript development:

```bash
# Watch TypeScript files
yarn build:ts --watch
```

### Testing Native Changes

```bash
# Rebuild only native module
yarn build:native

# Test without rebuilding TS
yarn test
```

### Debug Symbols

Keep debug symbols for troubleshooting:

```bash
# Linux/macOS
yarn build:native --debug

# Windows
node-gyp rebuild --debug --configuration=Debug
```

### Valgrind (Memory Leak Detection)

```bash
# Install valgrind
sudo apt-get install valgrind

# Run with valgrind
valgrind --leak-check=full node test.js
```

## References

- [node-gyp Documentation](https://github.com/nodejs/node-gyp)
- [Node-API Documentation](https://nodejs.org/api/n-api.html)
- [electron-rebuild](https://github.com/electron/electron-rebuild)
- [libssh Documentation](https://www.libssh.org/)
