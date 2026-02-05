# Quick Start Guide

Get up and running with libssh-node in 5 minutes.

## 1. Install libssh

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install libssh-dev
```

**macOS:**
```bash
brew install libssh
```

**Windows:**
```powershell
vcpkg install libssh:x64-windows
```

## 2. Install Dependencies

```bash
yarn install
```

## 3. Build the Project

```bash
# Build everything
yarn build

# Or build separately
yarn build:native  # C++ native module
yarn build:ts      # TypeScript
```

## 4. Run Tests

```bash
yarn test
```

## 5. Try an Example

### Basic Connection

Create `test-connection.ts`:

```typescript
import { SSHSession } from './lib';

async function main() {
  const session = new SSHSession({
    host: 'example.com',
    port: 22,
    user: 'your-username',
    autoDetectAgent: true
  });

  try {
    console.log('Connecting...');
    await session.connect();
    console.log('Connected!');

    console.log('Authenticating...');
    await session.authenticate({ useAgent: true });
    console.log('Authenticated!');

    console.log('Success!');
    await session.disconnect();
  } catch (err) {
    console.error('Error:', err);
    process.exit(1);
  }
}

main();
```

Run it:
```bash
npx ts-node test-connection.ts
```

### SSH Tunnel

Create `test-tunnel.ts`:

```typescript
import { SSHSession, SSHTunnel } from './lib';

async function main() {
  const session = new SSHSession({
    host: 'bastion.example.com',
    user: 'your-username',
    autoDetectAgent: true
  });

  try {
    await session.connect();
    await session.authenticate({ useAgent: true });

    const tunnel = new SSHTunnel({
      session,
      localPort: 3307,
      remoteHost: 'localhost',
      remotePort: 3306
    });

    await tunnel.start();
    console.log('Tunnel running on localhost:3307');
    console.log('Press Ctrl+C to stop');

    process.on('SIGINT', async () => {
      await tunnel.stop();
      await session.disconnect();
      process.exit(0);
    });
  } catch (err) {
    console.error('Error:', err);
    process.exit(1);
  }
}

main();
```

Run it:
```bash
npx ts-node test-tunnel.ts
```

## 6. For Electron

Rebuild for Electron:

```bash
yarn add --dev electron-rebuild
yarn electron-rebuild -v 31.0.0
```

## Common Issues

### libssh not found

```bash
# Linux
sudo ldconfig

# macOS
export LDFLAGS="-L$(brew --prefix libssh)/lib"
export CPPFLAGS="-I$(brew --prefix libssh)/include"
yarn rebuild
```

### Build fails

```bash
# Clean and rebuild
yarn clean
yarn rebuild
```

### Agent not detected

```bash
# Check agent is running
echo $SSH_AUTH_SOCK

# Start agent if needed
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_rsa
```

## Next Steps

- Read the full [README.md](README.md)
- Check out [examples/](examples/) for more usage patterns
- Read the [tunneling guide](docs/tunneling.md) for database tunnels
- Read the [agents guide](docs/agents.md) for 1Password/YubiKey setup

## Getting Help

- Check [IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md) for known issues
- See [docs/building.md](docs/building.md) for detailed build instructions
- Report issues on GitHub
