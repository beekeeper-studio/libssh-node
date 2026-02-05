import { AgentDetector } from './agent';
import { SSHConfigParser } from './config';

// Native module will be loaded
// eslint-disable-next-line @typescript-eslint/no-var-requires
const binding = require('../build/Release/libssh_node.node');

export interface SSHSessionOptions {
  host?: string;
  hostname?: string;
  port?: number;
  user?: string;
  configFile?: string;
  agentSocket?: string;
  timeout?: number;
  autoDetectAgent?: boolean;
}

export interface AuthOptions {
  username?: string;
  password?: string;
  useAgent?: boolean;
}

export class SSHSession {
  private session: typeof binding.SSHSession;

  constructor(options: SSHSessionOptions = {}) {
    // Auto-detect SSH agent if requested
    if (options.autoDetectAgent !== false && !options.agentSocket) {
      const agent = AgentDetector.detect();
      if (agent) {
        options.agentSocket = agent.socketPath;
      }
    }

    // Parse SSH config if host is provided
    if (options.host && !options.configFile) {
      const hostConfig = SSHConfigParser.findHostConfig(options.host);
      if (hostConfig) {
        options.hostname = hostConfig.hostname || options.host;
        options.port = options.port || hostConfig.port;
        options.user = options.user || hostConfig.user;
      }
    }

    this.session = new binding.SSHSession(options);
  }

  /**
   * Set a session option
   */
  setOption(name: string, value: string | number): void {
    this.session.setOption(name, value);
  }

  /**
   * Connect to the SSH server
   */
  async connect(): Promise<void> {
    return this.session.connect();
  }

  /**
   * Disconnect from the SSH server
   */
  async disconnect(): Promise<void> {
    return this.session.disconnect();
  }

  /**
   * Authenticate using password or agent
   */
  async authenticate(options: AuthOptions): Promise<void> {
    if (options.useAgent) {
      return this.session.authenticateAgent(options.username || null);
    } else if (options.password) {
      return this.session.authenticatePassword(options.username || null, options.password);
    } else {
      throw new Error('Either password or useAgent must be specified');
    }
  }

  /**
   * Parse SSH config file
   */
  parseConfig(configFile?: string): void {
    return this.session.parseConfig(configFile || null);
  }

  /**
   * Check if connected
   */
  isConnected(): boolean {
    return this.session.isConnected();
  }

  /**
   * Create a new channel
   */
  createChannel(): typeof binding.SSHChannel {
    return this.session.createChannel();
  }

  /**
   * Get the native session object (for advanced use)
   */
  getNativeSession(): typeof binding.SSHSession {
    return this.session;
  }
}
