# SSH Agent Configuration Guide

This guide explains how to configure and use SSH agents with libssh-node, including 1Password, YubiKey, and system agents.

## What is an SSH Agent?

An SSH agent holds your private keys in memory and provides them to SSH clients when needed. This allows you to:

- Avoid typing passwords repeatedly
- Keep private keys secure (never written to disk)
- Use hardware security keys (YubiKey)
- Centralize key management (1Password)

## Supported Agents

libssh-node auto-detects these agents in priority order:

1. **1Password SSH Agent** - Commercial password manager with SSH key support
2. **YubiKey Agent** - Hardware security key
3. **System SSH Agent** - Standard ssh-agent or ssh-pageant

## 1Password SSH Agent

### Setup on macOS

1. Install 1Password 8 or later
2. Enable SSH agent in 1Password settings:
   - Open 1Password
   - Go to Settings → Developer
   - Enable "Use the SSH agent"
   - Enable "Display key names when authorizing connections"

3. Add SSH keys to 1Password:
   - Click the "+" button
   - Select "SSH Key"
   - Import existing key or generate new one

### Setup on Linux

1. Install 1Password
2. Enable SSH agent in settings
3. Configure socket path:

```bash
# Add to ~/.bashrc or ~/.zshrc
export SSH_AUTH_SOCK=~/.1password/agent.sock
```

### Usage

```typescript
import { SSHSession } from 'libssh-node';

const session = new SSHSession({
  host: 'example.com',
  user: 'username',
  autoDetectAgent: true  // Will detect 1Password automatically
});

await session.connect();
await session.authenticate({ useAgent: true });
```

### Verification

```typescript
import { AgentDetector } from 'libssh-node';

const agent = AgentDetector.detect1Password();
if (agent) {
  console.log('1Password agent found:', agent.socketPath);
} else {
  console.log('1Password agent not available');
}
```

## YubiKey Agent

### Setup

1. Install yubikey-agent:

```bash
# macOS
brew install yubikey-agent

# Linux
go install github.com/FiloSottile/yubikey-agent@latest
```

2. Start the agent:

```bash
yubikey-agent -setup
```

3. Configure shell:

```bash
# Add to ~/.bashrc or ~/.zshrc
export SSH_AUTH_SOCK="${HOME}/.yubikey-agent.sock"
```

### Usage

```typescript
import { SSHSession } from 'libssh-node';

const session = new SSHSession({
  host: 'example.com',
  user: 'username',
  autoDetectAgent: true  // Will detect YubiKey automatically
});

await session.connect();
await session.authenticate({ useAgent: true });
```

### Verification

```typescript
import { AgentDetector } from 'libssh-node';

const agent = AgentDetector.detectYubiKey();
if (agent) {
  console.log('YubiKey agent found:', agent.socketPath);
} else {
  console.log('YubiKey agent not available');
}
```

## System SSH Agent

### Setup on Linux/macOS

1. Start ssh-agent:

```bash
eval "$(ssh-agent -s)"
```

2. Add keys:

```bash
ssh-add ~/.ssh/id_rsa
ssh-add ~/.ssh/id_ed25519
```

3. Make permanent (add to ~/.bashrc or ~/.zshrc):

```bash
if [ -z "$SSH_AUTH_SOCK" ]; then
  eval "$(ssh-agent -s)"
fi
```

### Setup on Windows

Windows 10/11 includes OpenSSH with ssh-agent:

1. Start the service:

```powershell
Get-Service -Name ssh-agent | Set-Service -StartupType Automatic
Start-Service ssh-agent
```

2. Add keys:

```powershell
ssh-add $HOME\.ssh\id_rsa
```

### Usage

```typescript
import { SSHSession } from 'libssh-node';

const session = new SSHSession({
  host: 'example.com',
  user: 'username',
  autoDetectAgent: true  // Will use SSH_AUTH_SOCK
});

await session.connect();
await session.authenticate({ useAgent: true });
```

## Agent Detection

### Automatic Detection

```typescript
import { AgentDetector } from 'libssh-node';

// Get the best available agent
const agent = AgentDetector.detect();
if (agent) {
  console.log(`Using ${agent.type} agent at ${agent.socketPath}`);
}
```

### Detect All Agents

```typescript
import { AgentDetector } from 'libssh-node';

const agents = AgentDetector.detectAll();
console.log(`Found ${agents.length} agent(s):`);

agents.forEach(agent => {
  console.log(`  - ${agent.type}: ${agent.socketPath}`);
});
```

### Manual Agent Selection

```typescript
import { SSHSession, AgentDetector } from 'libssh-node';

const agents = AgentDetector.detectAll();

// Let user choose
const selectedAgent = agents[0];  // or user selection

const session = new SSHSession({
  host: 'example.com',
  user: 'username',
  agentSocket: selectedAgent.socketPath,
  autoDetectAgent: false  // Disable auto-detection
});

await session.connect();
await session.authenticate({ useAgent: true });
```

## Custom Agent Socket

If you have an agent at a custom location:

```typescript
const session = new SSHSession({
  host: 'example.com',
  user: 'username',
  agentSocket: '/custom/path/to/agent.sock',
  autoDetectAgent: false
});

await session.connect();
await session.authenticate({ useAgent: true });
```

## Troubleshooting

### Agent Not Detected

1. Check if agent is running:

```bash
echo $SSH_AUTH_SOCK
```

2. For 1Password:
   - Check settings → Developer → SSH agent is enabled
   - Restart 1Password

3. For YubiKey:
   - Check yubikey-agent is running
   - Verify socket path exists

4. For system agent:
   - Run `ssh-add -l` to list keys
   - Start agent if needed: `eval "$(ssh-agent -s)"`

### Permission Denied

1. Check key is added to agent:

```bash
ssh-add -l
```

2. Verify key is authorized on server:

```bash
# On remote server
cat ~/.ssh/authorized_keys
```

3. Check key permissions:

```bash
chmod 600 ~/.ssh/id_rsa
chmod 644 ~/.ssh/id_rsa.pub
```

### Wrong Agent Used

Explicitly specify the agent:

```typescript
const session = new SSHSession({
  host: 'example.com',
  user: 'username',
  agentSocket: '/specific/agent.sock',
  autoDetectAgent: false
});
```

### Multiple Agents Conflict

Disable auto-detection and specify the agent:

```typescript
import { AgentDetector } from 'libssh-node';

const onePasswordAgent = AgentDetector.detect1Password();

const session = new SSHSession({
  host: 'example.com',
  user: 'username',
  agentSocket: onePasswordAgent.socketPath,
  autoDetectAgent: false
});
```

## Security Best Practices

1. **Use Agent Forwarding Carefully**: Only forward to trusted servers
2. **Lock Screen**: Agents remain active when screen is locked
3. **Key Management**: Regularly rotate SSH keys
4. **Hardware Keys**: Use YubiKey for highest security
5. **Timeout**: Configure agent timeout to auto-lock

## Agent Configuration Examples

### 1Password with Multiple Keys

```typescript
// 1Password can hold multiple keys
// It will prompt which key to use
const session = new SSHSession({
  host: 'example.com',
  user: 'username',
  autoDetectAgent: true
});

await session.connect();
await session.authenticate({ useAgent: true });
// 1Password will show a prompt to select key
```

### YubiKey with PIN

```typescript
// YubiKey requires physical touch and may require PIN
const session = new SSHSession({
  host: 'example.com',
  user: 'username',
  autoDetectAgent: true
});

await session.connect();
await session.authenticate({ useAgent: true });
// YubiKey will prompt for touch/PIN
```

### Fallback to Password

```typescript
import { SSHSession, AgentDetector } from 'libssh-node';

const session = new SSHSession({
  host: 'example.com',
  user: 'username',
  autoDetectAgent: true
});

await session.connect();

// Try agent first
const agent = AgentDetector.detect();
if (agent) {
  try {
    await session.authenticate({ useAgent: true });
    console.log('Authenticated with agent');
  } catch (err) {
    console.log('Agent auth failed, falling back to password');
    await session.authenticate({
      username: 'username',
      password: 'password'
    });
  }
} else {
  // No agent, use password
  await session.authenticate({
    username: 'username',
    password: 'password'
  });
}
```

## Integration with Beekeeper Studio

```typescript
class SSHConfig {
  static async getAgentInfo(): Promise<AgentInfo | null> {
    const agents = AgentDetector.detectAll();

    // Prefer 1Password for better UX
    const onePassword = agents.find(a => a.type === 'onepassword');
    if (onePassword) return onePassword;

    // Then YubiKey for security
    const yubikey = agents.find(a => a.type === 'yubikey');
    if (yubikey) return yubikey;

    // Finally system agent
    const system = agents.find(a => a.type === 'system');
    if (system) return system;

    return null;
  }

  static async createSession(config: ConnectionConfig): Promise<SSHSession> {
    const agent = await this.getAgentInfo();

    return new SSHSession({
      host: config.ssh.host,
      port: config.ssh.port,
      user: config.ssh.user,
      agentSocket: agent?.socketPath,
      autoDetectAgent: !agent  // Only auto-detect if we didn't find one
    });
  }
}
```

## References

- [1Password SSH Agent](https://developer.1password.com/docs/ssh/)
- [YubiKey Agent](https://github.com/FiloSottile/yubikey-agent)
- [OpenSSH Agent](https://www.ssh.com/academy/ssh/agent)
