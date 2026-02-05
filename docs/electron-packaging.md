# Bundling libssh with Electron Applications

This guide explains how to bundle libssh with your Electron application so end users don't need to install it separately.

## Overview

When distributing your Electron app:
- **Development**: Developers install libssh system-wide (`libssh-dev`)
- **Distribution**: Bundle libssh with your app package

This approach provides:
- ✅ Easy development setup
- ✅ No external dependencies for end users
- ✅ Consistent libssh version across all installations
- ✅ Simplified installation for users

## Table of Contents

- [Prerequisites](#prerequisites)
- [Platform-Specific Instructions](#platform-specific-instructions)
  - [Linux (AppImage, deb, rpm)](#linux-appimage-deb-rpm)
  - [macOS (dmg, pkg)](#macos-dmg-pkg)
  - [Windows (exe, portable)](#windows-exe-portable)
- [electron-builder Configuration](#electron-builder-configuration)
- [Testing](#testing)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### Install electron-builder

```bash
npm install --save-dev electron-builder
# or
yarn add --dev electron-builder
```

### Development Setup

Ensure libssh is installed on your development machine:

```bash
# Linux
sudo apt-get install libssh-dev

# macOS
brew install libssh

# Windows
vcpkg install libssh:x64-windows
```

## Platform-Specific Instructions

### Linux (AppImage, deb, rpm)

#### 1. Find libssh Location

```bash
# Find libssh shared library
ldconfig -p | grep libssh

# Common locations:
# /usr/lib/x86_64-linux-gnu/libssh.so.4
# /usr/lib/libssh.so.4
```

#### 2. Configure electron-builder

Create or update `electron-builder.yml`:

```yaml
appId: com.yourcompany.yourapp
productName: YourApp

linux:
  target:
    - AppImage
    - deb
    - rpm
  category: Development

  # Bundle libssh
  extraFiles:
    - from: /usr/lib/x86_64-linux-gnu/libssh.so.4
      to: lib/libssh.so.4
    - from: /usr/lib/x86_64-linux-gnu/libssh.so.4.9.6
      to: lib/libssh.so.4.9.6

  # Set RPATH so app finds bundled libssh
  asarUnpack:
    - "**/*.node"
```

#### 3. Update package.json

```json
{
  "scripts": {
    "build:linux": "electron-builder --linux"
  },
  "build": {
    "linux": {
      "extraFiles": [
        {
          "from": "/usr/lib/x86_64-linux-gnu/libssh.so.4",
          "to": "lib/libssh.so.4"
        }
      ]
    }
  }
}
```

#### 4. Set Library Path at Runtime

In your Electron main process:

```javascript
// main.js or main.ts
import { app } from 'electron';
import path from 'path';
import { spawn } from 'child_process';

// Add bundled lib directory to library path
if (process.platform === 'linux') {
  const libPath = path.join(process.resourcesPath, 'lib');
  process.env.LD_LIBRARY_PATH = `${libPath}:${process.env.LD_LIBRARY_PATH || ''}`;

  // For AppImage, might need to restart with updated LD_LIBRARY_PATH
  if (process.env.APPIMAGE && !process.env.RESTARTED) {
    const env = { ...process.env, RESTARTED: '1' };
    spawn(process.env.APPIMAGE, process.argv.slice(1), {
      env,
      detached: true,
      stdio: 'inherit'
    }).unref();
    app.quit();
    return;
  }
}

app.whenReady().then(() => {
  // Your app initialization
});
```

### macOS (dmg, pkg)

#### 1. Find libssh Location

```bash
# Find libssh
brew list libssh

# Common locations:
# /opt/homebrew/lib/libssh.4.dylib (Apple Silicon)
# /usr/local/lib/libssh.4.dylib (Intel)
```

#### 2. Configure electron-builder

```yaml
appId: com.yourcompany.yourapp
productName: YourApp

mac:
  target:
    - dmg
    - zip
  category: public.app-category.developer-tools
  hardenedRuntime: true
  gatekeeperAssess: false
  entitlements: build/entitlements.mac.plist

  # Bundle libssh
  extraFiles:
    - from: /opt/homebrew/lib/libssh.4.dylib
      to: Frameworks/libssh.4.dylib
    - from: /opt/homebrew/lib/libssh.4.9.6.dylib
      to: Frameworks/libssh.4.9.6.dylib
```

#### 3. Fix Library Install Names

Create a post-build script `scripts/fix-macos-libs.sh`:

```bash
#!/bin/bash

# Path to your app bundle
APP_PATH="dist/mac/YourApp.app"

# Fix install names for libssh
install_name_tool -change \
  /opt/homebrew/lib/libssh.4.dylib \
  @rpath/libssh.4.dylib \
  "$APP_PATH/Contents/Resources/app.asar.unpacked/node_modules/libssh-node/build/Release/libssh_node.node"

# Add rpath to Frameworks directory
install_name_tool -add_rpath \
  @loader_path/../../Frameworks \
  "$APP_PATH/Contents/Resources/app.asar.unpacked/node_modules/libssh-node/build/Release/libssh_node.node"

echo "Fixed macOS library paths"
```

Make it executable:
```bash
chmod +x scripts/fix-macos-libs.sh
```

#### 4. Update package.json

```json
{
  "scripts": {
    "build:mac": "electron-builder --mac && ./scripts/fix-macos-libs.sh"
  },
  "build": {
    "mac": {
      "extraFiles": [
        {
          "from": "/opt/homebrew/lib/libssh.4.dylib",
          "to": "Frameworks/libssh.4.dylib"
        }
      ]
    }
  }
}
```

#### 5. Entitlements (for hardened runtime)

Create `build/entitlements.mac.plist`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>com.apple.security.cs.allow-jit</key>
  <true/>
  <key>com.apple.security.cs.allow-unsigned-executable-memory</key>
  <true/>
  <key>com.apple.security.cs.allow-dyld-environment-variables</key>
  <true/>
</dict>
</plist>
```

### Windows (exe, portable)

#### 1. Find libssh DLLs

```powershell
# If using vcpkg
dir C:\vcpkg\installed\x64-windows\bin\ssh.dll

# Common locations:
# C:\vcpkg\installed\x64-windows\bin\ssh.dll
# C:\Program Files\libssh\bin\ssh.dll
```

#### 2. Configure electron-builder

```yaml
appId: com.yourcompany.yourapp
productName: YourApp

win:
  target:
    - nsis
    - portable

  # Bundle libssh DLLs
  extraFiles:
    - from: C:/vcpkg/installed/x64-windows/bin/ssh.dll
      to: ssh.dll
    - from: C:/vcpkg/installed/x64-windows/bin/zlib1.dll
      to: zlib1.dll
    - from: C:/vcpkg/installed/x64-windows/bin/crypto-3-x64.dll
      to: crypto-3-x64.dll
    - from: C:/vcpkg/installed/x64-windows/bin/ssl-3-x64.dll
      to: ssl-3-x64.dll
```

#### 3. Update package.json

```json
{
  "scripts": {
    "build:win": "electron-builder --win"
  },
  "build": {
    "win": {
      "extraFiles": [
        {
          "from": "C:/vcpkg/installed/x64-windows/bin/ssh.dll",
          "to": "ssh.dll"
        },
        {
          "from": "C:/vcpkg/installed/x64-windows/bin/zlib1.dll",
          "to": "zlib1.dll"
        }
      ]
    }
  }
}
```

#### 4. Set DLL Search Path

In your Electron main process:

```javascript
// main.js or main.ts
import { app } from 'electron';
import path from 'path';

if (process.platform === 'win32') {
  // Add resources directory to DLL search path
  const dllPath = path.join(process.resourcesPath);

  // Windows 10+ supports SetDefaultDllDirectories
  process.env.PATH = `${dllPath};${process.env.PATH}`;
}

app.whenReady().then(() => {
  // Your app initialization
});
```

## electron-builder Configuration

### Complete Example

Here's a complete `electron-builder.yml` for all platforms:

```yaml
appId: com.beekeeperstudio.studio
productName: Beekeeper Studio

directories:
  output: dist
  buildResources: build

files:
  - "**/*"
  - "!**/*.ts"
  - "!**/*.map"

asarUnpack:
  - "**/*.node"

linux:
  target:
    - AppImage
    - deb
    - rpm
  category: Development
  extraFiles:
    - from: /usr/lib/x86_64-linux-gnu/libssh.so.4
      to: lib/libssh.so.4

mac:
  target:
    - dmg
  category: public.app-category.developer-tools
  hardenedRuntime: true
  extraFiles:
    - from: /opt/homebrew/lib/libssh.4.dylib
      to: Frameworks/libssh.4.dylib

win:
  target:
    - nsis
    - portable
  extraFiles:
    - from: C:/vcpkg/installed/x64-windows/bin/ssh.dll
      to: ssh.dll
    - from: C:/vcpkg/installed/x64-windows/bin/zlib1.dll
      to: zlib1.dll
```

### package.json Scripts

```json
{
  "scripts": {
    "postinstall": "electron-builder install-app-deps",
    "build": "electron-builder",
    "build:mac": "electron-builder --mac",
    "build:linux": "electron-builder --linux",
    "build:win": "electron-builder --win",
    "build:all": "electron-builder -mwl"
  }
}
```

## Testing

### Test Bundled Libraries

#### Linux

```bash
# Build AppImage
npm run build:linux

# Extract and check libraries
./dist/YourApp-1.0.0.AppImage --appimage-extract
ls squashfs-root/lib/

# Check if libssh is found
ldd squashfs-root/resources/app.asar.unpacked/node_modules/libssh-node/build/Release/libssh_node.node
```

#### macOS

```bash
# Build dmg
npm run build:mac

# Mount dmg and check
hdiutil attach dist/YourApp-1.0.0.dmg
cd /Volumes/YourApp/
otool -L YourApp.app/Contents/Resources/app.asar.unpacked/node_modules/libssh-node/build/Release/libssh_node.node

# Should show @rpath/libssh.4.dylib
```

#### Windows

```bash
# Build installer
npm run build:win

# Install and check
# Open PowerShell in installation directory
dir *.dll

# Check DLL dependencies
# Download Dependencies.exe (Dependency Walker alternative)
Dependencies.exe -imports YourApp.exe
```

### Runtime Testing

Create a test script to verify libssh works:

```javascript
// test-libssh.js
const { SSHSession } = require('libssh-node');

async function test() {
  try {
    const session = new SSHSession({
      host: 'example.com',
      port: 22,
      user: 'test'
    });

    console.log('✓ libssh-node loaded successfully');
    console.log('✓ Session created');
    return true;
  } catch (error) {
    console.error('✗ Error:', error.message);
    return false;
  }
}

test().then(success => {
  process.exit(success ? 0 : 1);
});
```

Run in your packaged app:
```bash
# Linux
./dist/linux-unpacked/YourApp --eval "require('./test-libssh.js')"

# macOS
./dist/mac/YourApp.app/Contents/MacOS/YourApp --eval "require('./test-libssh.js')"

# Windows
.\dist\win-unpacked\YourApp.exe --eval "require('./test-libssh.js')"
```

## Troubleshooting

### Linux: "libssh.so.4: cannot open shared object file"

**Problem:** App can't find bundled libssh.

**Solutions:**
1. Check `LD_LIBRARY_PATH` is set correctly
2. Verify libssh is in `resources/lib/` directory
3. Check file permissions: `chmod 755 lib/libssh.so.4`
4. For AppImage, ensure restart logic works

### macOS: "Library not loaded: /opt/homebrew/lib/libssh.4.dylib"

**Problem:** Library paths not fixed.

**Solutions:**
1. Run `fix-macos-libs.sh` script after build
2. Check install names: `otool -L libssh_node.node`
3. Verify rpath: `otool -l libssh_node.node | grep RPATH`
4. Ensure libssh is copied to `Frameworks/` directory

### Windows: "ssh.dll not found"

**Problem:** DLL search path incorrect.

**Solutions:**
1. Verify DLLs are in application root
2. Check PATH environment variable
3. Include ALL dependencies (zlib, OpenSSL, etc.)
4. Use Dependency Walker to find missing DLLs

### All Platforms: "Module did not self-register"

**Problem:** Native module built for wrong Electron version.

**Solutions:**
1. Rebuild for Electron: `electron-rebuild`
2. Check Electron version matches: `electron --version`
3. Clear build cache: `rm -rf node_modules/libssh-node/build`
4. Rebuild: `cd node_modules/libssh-node && npm run build:native`

### Check Native Module ABI

```bash
# Should match Electron's Node version
node -e "console.log(process.versions.modules)"
# vs
./node_modules/.bin/electron -e "console.log(process.versions.modules)"
```

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Build and Release

on:
  push:
    tags:
      - 'v*'

jobs:
  build:
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]

    runs-on: ${{ matrix.os }}

    steps:
      - uses: actions/checkout@v4

      - uses: actions/setup-node@v4
        with:
          node-version: '20'

      - name: Install libssh (Ubuntu)
        if: runner.os == 'Linux'
        run: sudo apt-get install -y libssh-dev

      - name: Install libssh (macOS)
        if: runner.os == 'macOS'
        run: brew install libssh

      - name: Install libssh (Windows)
        if: runner.os == 'Windows'
        run: vcpkg install libssh:x64-windows

      - name: Install dependencies
        run: npm install

      - name: Build Electron app
        run: npm run build
        env:
          GH_TOKEN: ${{ secrets.GITHUB_TOKEN }}

      - name: Upload artifacts
        uses: actions/upload-artifact@v3
        with:
          name: ${{ runner.os }}-build
          path: dist/*
```

## Best Practices

1. **Version Locking**: Pin libssh version in your build scripts
2. **Test Regularly**: Test packaged apps on clean VMs
3. **Document Versions**: Keep track of bundled library versions
4. **Security Updates**: Monitor libssh CVEs and update promptly
5. **Minimize Dependencies**: Only bundle required libraries
6. **Code Signing**: Sign all binaries including bundled libraries
7. **Size Optimization**: Strip debug symbols from release builds

## Additional Resources

- [electron-builder Documentation](https://www.electron.build/)
- [Electron Native Modules](https://www.electronjs.org/docs/latest/tutorial/using-native-node-modules)
- [libssh Documentation](https://www.libssh.org/)
- [Beekeeper Studio](https://www.beekeeperstudio.io/)

## Getting Help

If you encounter issues:
1. Check this documentation
2. Review [building.md](building.md) for build setup
3. Search existing [GitHub issues](https://github.com/beekeeper-studio/libssh-node/issues)
4. Create a new issue with:
   - Platform and version
   - Electron version
   - Build logs
   - Error messages
