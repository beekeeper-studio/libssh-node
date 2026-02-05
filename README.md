# libssh-node

Node.js native wrapper for libssh with TypeScript support, designed for Electron applications like Beekeeper Studio.

## Features

- **SSH Client Connections**: Connect to SSH servers with multiple authentication methods
- **SSH Agent Support**: Auto-detection of 1Password, YubiKey, and system SSH agents
- **SSH Config Files**: Parse and use SSH config files (~/.ssh/config)
- **Port Forwarding/Tunneling**: Create SSH tunnels for secure database connections
- **TypeScript Support**: Full TypeScript type definitions
- **Async API**: Promise-based API using Node-API async workers
- **SFTP Support**: File transfer operations (coming soon)

## Installation

### Prerequisites

Install libssh on your system:

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
vcpkg install libssh
```

### Install the package

```bash
yarn add libssh-node
# or
npm install libssh-node
```

### For Electron

#### Development

Rebuild for your Electron version:

```bash
yarn rebuild --runtime=electron --target=31.0.0
# or
npm run rebuild -- --runtime=electron --target=31.0.0
```

#### Distribution

When packaging your Electron app for distribution, you need to bundle libssh with your application. See the complete guide:

📦 **[Electron Packaging Guide](docs/electron-packaging.md)** - How to bundle libssh with your Electron app

This ensures end users don't need to install libssh separately.

## Quick Start

### Basic Connection with Password

```typescript
import { SSHSession } from 'libssh-node';

const session = new SSHSession({
  host: 'example.com',
  port: 22,
  user: 'username'
});

await session.connect();
await session.authenticate({
  username: 'username',
  password: 'password'
});

await session.disconnect();
```

### Connection with SSH Agent

```typescript
import { SSHSession } from 'libssh-node';

const session = new SSHSession({
  host: 'example.com',
  port: 22,
  user: 'username',
  autoDetectAgent: true  // Auto-detect 1Password, YubiKey, or system agent
});

await session.connect();
await session.authenticate({ useAgent: true });
await session.disconnect();
```

### SSH Tunnel for Database Connection

```typescript
import { SSHSession, SSHTunnel } from 'libssh-node';

const session = new SSHSession({
  host: 'ssh-server.com',
  user: 'username',
  autoDetectAgent: true
});

await session.connect();
await session.authenticate({ useAgent: true });

// Create tunnel: localhost:3307 -> remote MySQL on localhost:3306
const tunnel = new SSHTunnel({
  session,
  localHost: '127.0.0.1',
  localPort: 3307,
  remoteHost: 'localhost',
  remotePort: 3306
});

await tunnel.start();

// Now connect your database client to localhost:3307
// The tunnel will forward all traffic through SSH

// When done:
await tunnel.stop();
await session.disconnect();
```

### Detect Available SSH Agents

```typescript
import { AgentDetector } from 'libssh-node';

const agents = AgentDetector.detectAll();
console.log('Available agents:', agents);
// [
//   { type: 'onepassword', socketPath: '/path/to/1password/agent.sock' },
//   { type: 'system', socketPath: '/tmp/ssh-agent.sock' }
// ]

// Or get the best available agent:
const agent = AgentDetector.detect();
```

### Parse SSH Config

```typescript
import { SSHConfigParser } from 'libssh-node';

// Parse ~/.ssh/config
const hosts = SSHConfigParser.parse();

// Find specific host
const config = SSHConfigParser.findHostConfig('myserver');
console.log(config.hostname, config.port, config.user);
```

## API Documentation

### SSHSession

**Constructor Options:**
- `host?: string` - SSH server hostname
- `port?: number` - SSH server port (default: 22)
- `user?: string` - SSH username
- `configFile?: string` - Path to SSH config file
- `agentSocket?: string` - Custom SSH agent socket path
- `timeout?: number` - Connection timeout in milliseconds
- `autoDetectAgent?: boolean` - Auto-detect SSH agents (default: true)

**Methods:**
- `connect(): Promise<void>` - Connect to SSH server
- `disconnect(): Promise<void>` - Disconnect from server
- `authenticate(options: AuthOptions): Promise<void>` - Authenticate
- `isConnected(): boolean` - Check connection status
- `createChannel()` - Create a new SSH channel

### SSHTunnel

**Constructor Options:**
- `session: SSHSession` - Connected SSH session
- `localHost?: string` - Local bind address (default: '127.0.0.1')
- `localPort?: number` - Local port (default: 0 = auto-assign)
- `remoteHost: string` - Remote host to forward to
- `remotePort: number` - Remote port to forward to

**Methods:**
- `start(): Promise<void>` - Start the tunnel
- `stop(): Promise<void>` - Stop the tunnel
- `getLocalAddress(): { host: string; port: number } | null` - Get local address
- `isRunning(): boolean` - Check if tunnel is running
- `getActiveConnectionCount(): number` - Get number of active connections

### AgentDetector

**Static Methods:**
- `detect(): AgentInfo | null` - Get best available agent
- `detectAll(): AgentInfo[]` - Get all available agents
- `detect1Password(): AgentInfo | null` - Detect 1Password agent
- `detectYubiKey(): AgentInfo | null` - Detect YubiKey agent
- `detectSystemAgent(): AgentInfo | null` - Detect system agent

### SSHConfigParser

**Static Methods:**
- `parse(configPath?: string): SSHConfigHost[]` - Parse SSH config file
- `findHostConfig(hostname: string, configPath?: string): SSHConfigHost | null` - Find host config

## Documentation

### Guides

- **[Tunneling Guide](docs/tunneling.md)** - Complete guide to SSH tunneling for database connections
- **[SSH Agents Guide](docs/agents.md)** - Setting up 1Password, YubiKey, and system SSH agents
- **[Building Guide](docs/building.md)** - Platform-specific build instructions
- **[Electron Packaging Guide](docs/electron-packaging.md)** - Bundle libssh with your Electron app

### Examples

See the [examples/](examples/) directory for code examples:

- [basic-connection.ts](examples/basic-connection.ts) - Simple connection
- [ssh-tunnel.ts](examples/ssh-tunnel.ts) - Database tunnel setup
- [agent-authentication.ts](examples/agent-authentication.ts) - Using SSH agents
- [password-auth.ts](examples/password-auth.ts) - Password authentication

## Building from Source

```bash
# Install dependencies
yarn install

# Build native module
yarn build:native

# Build TypeScript
yarn build:ts

# Run tests
yarn test
```

## Platform Support

- Linux (x64, arm64)
- macOS (x64, arm64)
- Windows (x64)

## License

MIT

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for details.

## Troubleshooting

### libssh not found during build

Make sure libssh is installed and in your system's library path:

**Linux:** `sudo ldconfig` after installing libssh-dev
**macOS:** Ensure Homebrew's lib directory is in your path
**Windows:** Set the library path in vcpkg

### Electron rebuild fails

Ensure you have the correct build tools:

**Linux:** `build-essential`, `python3`
**macOS:** Xcode Command Line Tools
**Windows:** Visual Studio Build Tools

Then rebuild:
```bash
yarn rebuild --runtime=electron --target=YOUR_ELECTRON_VERSION
```

### SSH agent not detected

1. Check if your agent is running: `echo $SSH_AUTH_SOCK`
2. For 1Password, ensure SSH agent is enabled in settings
3. For YubiKey, ensure yubikey-agent is running

## Support

For issues and questions:
- GitHub Issues: https://github.com/beekeeper-studio/libssh-node/issues
- Documentation: https://docs.beekeeperstudio.io
