# SSH Tunneling Guide

This guide explains how to use libssh-node to create SSH tunnels for secure database connections in Beekeeper Studio and other applications.

## What is SSH Tunneling?

SSH tunneling (port forwarding) creates an encrypted connection between your local machine and a remote server, allowing you to securely access services that are not directly accessible.

### Use Cases

1. **Database Connections**: Access remote databases that only accept localhost connections
2. **Firewall Bypass**: Connect to services behind firewalls
3. **Encryption**: Add encryption to protocols that don't support it natively
4. **Multi-hop Access**: Access services through jump hosts

## Basic SSH Tunnel

The simplest tunnel forwards a local port to a remote port:

```typescript
import { SSHSession, SSHTunnel } from 'libssh-node';

// Create SSH session
const session = new SSHSession({
  host: 'bastion.example.com',
  port: 22,
  user: 'username',
  autoDetectAgent: true
});

// Connect and authenticate
await session.connect();
await session.authenticate({ useAgent: true });

// Create tunnel
const tunnel = new SSHTunnel({
  session,
  localHost: '127.0.0.1',
  localPort: 3307,        // Local MySQL port
  remoteHost: 'localhost', // On the remote server
  remotePort: 3306        // Remote MySQL port
});

await tunnel.start();

// Now connect to localhost:3307
// All traffic will be forwarded to the remote MySQL server
```

## Database Tunneling Examples

### MySQL/MariaDB

```typescript
const tunnel = new SSHTunnel({
  session,
  localHost: '127.0.0.1',
  localPort: 3307,
  remoteHost: 'localhost',
  remotePort: 3306
});

await tunnel.start();

// Connect with mysql client:
// mysql -h 127.0.0.1 -P 3307 -u dbuser -p
```

### PostgreSQL

```typescript
const tunnel = new SSHTunnel({
  session,
  localHost: '127.0.0.1',
  localPort: 5433,
  remoteHost: 'localhost',
  remotePort: 5432
});

await tunnel.start();

// Connect with psql:
// psql -h 127.0.0.1 -p 5433 -U dbuser
```

### MongoDB

```typescript
const tunnel = new SSHTunnel({
  session,
  localHost: '127.0.0.1',
  localPort: 27018,
  remoteHost: 'localhost',
  remotePort: 27017
});

await tunnel.start();

// Connect with mongo client:
// mongo mongodb://127.0.0.1:27018/mydb
```

### Redis

```typescript
const tunnel = new SSHTunnel({
  session,
  localHost: '127.0.0.1',
  localPort: 6380,
  remoteHost: 'localhost',
  remotePort: 6379
});

await tunnel.start();

// Connect with redis-cli:
// redis-cli -h 127.0.0.1 -p 6380
```

## Advanced Tunneling

### Auto-assign Local Port

Let the system choose an available port:

```typescript
const tunnel = new SSHTunnel({
  session,
  localPort: 0, // 0 means auto-assign
  remoteHost: 'localhost',
  remotePort: 3306
});

await tunnel.start();

const address = tunnel.getLocalAddress();
console.log(`Connect to localhost:${address.port}`);
```

### Multiple Tunnels

Create multiple tunnels on the same SSH session:

```typescript
// MySQL tunnel
const mysqlTunnel = new SSHTunnel({
  session,
  localPort: 3307,
  remoteHost: 'localhost',
  remotePort: 3306
});

// PostgreSQL tunnel
const pgTunnel = new SSHTunnel({
  session,
  localPort: 5433,
  remoteHost: 'localhost',
  remotePort: 5432
});

await Promise.all([
  mysqlTunnel.start(),
  pgTunnel.start()
]);

// Both tunnels are now active
```

### Remote Database Server

Forward to a different host accessible from the SSH server:

```typescript
const tunnel = new SSHTunnel({
  session,
  localPort: 3307,
  remoteHost: 'db-server.internal', // Different host
  remotePort: 3306
});

await tunnel.start();

// Connects to db-server.internal:3306 through the SSH server
```

### Connection Monitoring

Monitor active connections through the tunnel:

```typescript
const tunnel = new SSHTunnel({
  session,
  localPort: 3307,
  remoteHost: 'localhost',
  remotePort: 3306
});

await tunnel.start();

// Check active connections periodically
setInterval(() => {
  const count = tunnel.getActiveConnectionCount();
  console.log(`Active connections: ${count}`);
}, 5000);
```

## Error Handling

### Connection Failures

```typescript
try {
  await tunnel.start();
} catch (err) {
  if (err.message.includes('EADDRINUSE')) {
    console.error('Local port is already in use');
  } else if (err.message.includes('Connection refused')) {
    console.error('Cannot connect to remote service');
  } else {
    console.error('Tunnel error:', err);
  }
}
```

### Graceful Shutdown

```typescript
async function shutdown() {
  console.log('Shutting down tunnel...');

  try {
    await tunnel.stop();
    await session.disconnect();
    console.log('Tunnel closed successfully');
  } catch (err) {
    console.error('Error during shutdown:', err);
  }
}

process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);
```

### Reconnection

```typescript
async function createResilientTunnel() {
  let tunnel = null;

  async function connect() {
    const session = new SSHSession({
      host: 'example.com',
      user: 'username',
      autoDetectAgent: true
    });

    await session.connect();
    await session.authenticate({ useAgent: true });

    tunnel = new SSHTunnel({
      session,
      localPort: 3307,
      remoteHost: 'localhost',
      remotePort: 3306
    });

    await tunnel.start();
  }

  // Initial connection
  await connect();

  // Monitor connection
  setInterval(async () => {
    if (!tunnel.isRunning()) {
      console.log('Tunnel down, reconnecting...');
      try {
        await connect();
        console.log('Tunnel restored');
      } catch (err) {
        console.error('Reconnection failed:', err);
      }
    }
  }, 10000);
}
```

## Integration with Beekeeper Studio

### Basic Integration

```typescript
class DatabaseConnection {
  private session: SSHSession | null = null;
  private tunnel: SSHTunnel | null = null;

  async connect(config: ConnectionConfig) {
    // If SSH is required
    if (config.ssh) {
      this.session = new SSHSession({
        host: config.ssh.host,
        port: config.ssh.port,
        user: config.ssh.user,
        autoDetectAgent: true
      });

      await this.session.connect();
      await this.session.authenticate({ useAgent: true });

      this.tunnel = new SSHTunnel({
        session: this.session,
        localPort: 0, // Auto-assign
        remoteHost: config.db.host,
        remotePort: config.db.port
      });

      await this.tunnel.start();

      const address = this.tunnel.getLocalAddress();

      // Update connection config to use tunnel
      config.db.host = address.host;
      config.db.port = address.port;
    }

    // Connect to database using updated config
    // ...
  }

  async disconnect() {
    if (this.tunnel) {
      await this.tunnel.stop();
    }
    if (this.session) {
      await this.session.disconnect();
    }
  }
}
```

### With Error Recovery

```typescript
class ResilientDatabaseConnection {
  private session: SSHSession | null = null;
  private tunnel: SSHTunnel | null = null;
  private reconnectTimer: NodeJS.Timeout | null = null;

  async connect(config: ConnectionConfig) {
    try {
      if (config.ssh) {
        await this.setupTunnel(config);
      }

      // Setup reconnection monitoring
      this.startMonitoring();

    } catch (err) {
      await this.cleanup();
      throw err;
    }
  }

  private async setupTunnel(config: ConnectionConfig) {
    this.session = new SSHSession({
      host: config.ssh.host,
      port: config.ssh.port,
      user: config.ssh.user,
      autoDetectAgent: true,
      timeout: 30000
    });

    await this.session.connect();
    await this.session.authenticate({ useAgent: true });

    this.tunnel = new SSHTunnel({
      session: this.session,
      localPort: 0,
      remoteHost: config.db.host,
      remotePort: config.db.port
    });

    await this.tunnel.start();
  }

  private startMonitoring() {
    this.reconnectTimer = setInterval(() => {
      if (this.session && !this.session.isConnected()) {
        this.handleDisconnection();
      }
    }, 5000);
  }

  private async handleDisconnection() {
    console.error('SSH connection lost');
    // Emit event to UI
    // Attempt reconnection
    // ...
  }

  async cleanup() {
    if (this.reconnectTimer) {
      clearInterval(this.reconnectTimer);
    }
    if (this.tunnel) {
      await this.tunnel.stop();
    }
    if (this.session) {
      await this.session.disconnect();
    }
  }
}
```

## Performance Tips

1. **Reuse SSH Sessions**: Create one session and multiple tunnels instead of multiple sessions
2. **Connection Pooling**: Keep tunnels open for frequently accessed databases
3. **Timeout Configuration**: Set appropriate timeouts based on network conditions
4. **Compression**: Enable SSH compression for slow connections (configure in SSH session)

## Security Considerations

1. **Agent Security**: Use SSH agents (1Password, YubiKey) instead of password authentication
2. **Key Management**: Never hardcode passwords or private keys
3. **Connection Limits**: Limit the number of active connections
4. **Audit Logging**: Log connection attempts and failures
5. **Timeout**: Always set connection timeouts to prevent hanging

## Troubleshooting

### Tunnel Won't Start

Check:
- SSH connection is active
- Local port is not in use
- Remote service is running
- Firewall rules allow the connection

### Connection Drops

Check:
- SSH keep-alive settings
- Network stability
- Firewall timeout settings
- Server connection limits

### Performance Issues

Check:
- Network latency
- Bandwidth limitations
- Server load
- Consider enabling compression

## References

- [libssh Documentation](https://www.libssh.org/)
- [SSH Port Forwarding](https://www.ssh.com/academy/ssh/tunneling-example)
- [Beekeeper Studio](https://www.beekeeperstudio.io/)
