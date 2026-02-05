import * as net from 'net';
import { SSHSession } from './session';
import { SSHChannel } from './channel';
import { SSHTunnelError } from './errors';

export interface TunnelOptions {
  session: SSHSession;
  localHost?: string;
  localPort?: number;
  remoteHost: string;
  remotePort: number;
}

interface ChannelMapping {
  channel: SSHChannel;
  socket: net.Socket;
}

export class SSHTunnel {
  private session: SSHSession;
  private localHost: string;
  private localPort: number;
  private remoteHost: string;
  private remotePort: number;
  private server: net.Server | null = null;
  private channels: Map<number, ChannelMapping> = new Map();
  private channelIdCounter = 0;

  constructor(options: TunnelOptions) {
    this.session = options.session;
    this.localHost = options.localHost || '127.0.0.1';
    this.localPort = options.localPort || 0; // 0 means auto-assign
    this.remoteHost = options.remoteHost;
    this.remotePort = options.remotePort;
  }

  /**
   * Start the SSH tunnel
   */
  async start(): Promise<void> {
    if (this.server) {
      throw new SSHTunnelError('Tunnel is already started');
    }

    if (!this.session.isConnected()) {
      throw new SSHTunnelError('SSH session is not connected');
    }

    return new Promise((resolve, reject) => {
      this.server = net.createServer((socket) => {
        this.handleConnection(socket).catch((err) => {
          console.error('Error handling connection:', err);
          socket.destroy();
        });
      });

      this.server.on('error', (err) => {
        reject(new SSHTunnelError(`Server error: ${err.message}`));
      });

      this.server.listen(this.localPort, this.localHost, () => {
        const address = this.server!.address() as net.AddressInfo;
        this.localPort = address.port;
        resolve();
      });
    });
  }

  /**
   * Stop the SSH tunnel
   */
  async stop(): Promise<void> {
    if (!this.server) {
      return;
    }

    // Close all active channels
    for (const [id, mapping] of this.channels.entries()) {
      try {
        mapping.socket.destroy();
        await mapping.channel.close();
      } catch (err) {
        console.error(`Error closing channel ${id}:`, err);
      }
    }
    this.channels.clear();

    // Close the server
    return new Promise((resolve, reject) => {
      this.server!.close((err) => {
        if (err) {
          reject(new SSHTunnelError(`Failed to close server: ${err.message}`));
        } else {
          this.server = null;
          resolve();
        }
      });
    });
  }

  /**
   * Get local address information
   */
  getLocalAddress(): { host: string; port: number } | null {
    if (!this.server || !this.server.listening) {
      return null;
    }

    const address = this.server.address() as net.AddressInfo;
    return {
      host: address.address,
      port: address.port
    };
  }

  /**
   * Handle new client connection
   */
  private async handleConnection(socket: net.Socket): Promise<void> {
    const channelId = this.channelIdCounter++;

    try {
      // Create SSH channel
      const nativeChannel = this.session.createChannel();
      const channel = new SSHChannel(nativeChannel);

      // Open forwarded channel
      await channel.requestForwardTcpIp(
        this.remoteHost,
        this.remotePort,
        socket.remoteAddress || '127.0.0.1',
        socket.remotePort || 0
      );

      // Store channel mapping
      this.channels.set(channelId, { channel, socket });

      // Set up bidirectional data forwarding
      this.setupDataForwarding(channelId, channel, socket);

    } catch (err) {
      console.error(`Failed to create channel ${channelId}:`, err);
      socket.destroy();
      this.channels.delete(channelId);
    }
  }

  /**
   * Set up bidirectional data forwarding between socket and channel
   */
  private setupDataForwarding(channelId: number, channel: SSHChannel, socket: net.Socket): void {
    const isForwarding = true;

    // Socket -> Channel
    socket.on('data', async (data) => {
      if (!isForwarding) return;

      try {
        await channel.write(data);
      } catch (err) {
        console.error(`Error writing to channel ${channelId}:`, err);
        this.closeConnection(channelId);
      }
    });

    // Channel -> Socket
    const readLoop = async () => {
      while (isForwarding && channel.isOpen()) {
        try {
          const data = await channel.read();
          if (data.length > 0) {
            socket.write(data);
          } else {
            // EOF
            break;
          }
        } catch (err) {
          console.error(`Error reading from channel ${channelId}:`, err);
          break;
        }
      }

      this.closeConnection(channelId);
    };

    readLoop();

    // Handle socket close
    socket.on('close', () => {
      this.closeConnection(channelId);
    });

    socket.on('error', (err) => {
      console.error(`Socket error for channel ${channelId}:`, err);
      this.closeConnection(channelId);
    });
  }

  /**
   * Close a specific connection
   */
  private async closeConnection(channelId: number): Promise<void> {
    const mapping = this.channels.get(channelId);
    if (!mapping) return;

    this.channels.delete(channelId);

    try {
      mapping.socket.destroy();
    } catch (err) {
      // Ignore errors when closing socket
    }

    try {
      await mapping.channel.close();
    } catch (err) {
      // Ignore errors when closing channel
    }
  }

  /**
   * Get number of active connections
   */
  getActiveConnectionCount(): number {
    return this.channels.size;
  }

  /**
   * Check if tunnel is running
   */
  isRunning(): boolean {
    return this.server !== null && this.server.listening;
  }
}
