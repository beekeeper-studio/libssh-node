// eslint-disable-next-line @typescript-eslint/no-var-requires
const binding = require('../build/Release/libssh_node.node');

export class SSHChannel {
  private channel: typeof binding.SSHChannel;

  constructor(nativeChannel: typeof binding.SSHChannel) {
    this.channel = nativeChannel;
  }

  /**
   * Open a session channel
   */
  async openSession(): Promise<void> {
    return this.channel.openSession();
  }

  /**
   * Execute a command on the remote host
   */
  async requestExec(command: string): Promise<void> {
    return this.channel.requestExec(command);
  }

  /**
   * Request TCP/IP port forwarding
   */
  async requestForwardTcpIp(
    remoteHost: string,
    remotePort: number,
    sourceHost?: string,
    sourcePort?: number
  ): Promise<void> {
    return this.channel.requestForwardTcpIp(remoteHost, remotePort, sourceHost, sourcePort);
  }

  /**
   * Read data from the channel
   */
  async read(maxBytes?: number): Promise<Buffer> {
    return this.channel.read(maxBytes);
  }

  /**
   * Write data to the channel
   */
  async write(data: Buffer): Promise<number> {
    return this.channel.write(data);
  }

  /**
   * Close the channel
   */
  async close(): Promise<void> {
    return this.channel.close();
  }

  /**
   * Check if channel is open
   */
  isOpen(): boolean {
    return this.channel.isOpen();
  }

  /**
   * Get the native channel object (for advanced use)
   */
  getNativeChannel(): typeof binding.SSHChannel {
    return this.channel;
  }
}
