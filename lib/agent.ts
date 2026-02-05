import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';

export interface AgentInfo {
  type: 'onepassword' | 'yubikey' | 'system';
  socketPath: string;
}

export class AgentDetector {
  /**
   * Detect available SSH agents in priority order:
   * 1. 1Password
   * 2. YubiKey
   * 3. System SSH_AUTH_SOCK
   */
  static detect(): AgentInfo | null {
    return this.detect1Password() || this.detectYubiKey() || this.detectSystemAgent();
  }

  /**
   * Detect 1Password SSH agent
   */
  static detect1Password(): AgentInfo | null {
    const platform = os.platform();
    let socketPath: string | null = null;

    if (platform === 'darwin') {
      // macOS: ~/Library/Group Containers/2BUA8C4S2C.com.1password/t/agent.sock
      const groupContainers = path.join(os.homedir(), 'Library', 'Group Containers');
      const onePasswordDir = '2BUA8C4S2C.com.1password';
      socketPath = path.join(groupContainers, onePasswordDir, 't', 'agent.sock');
    } else if (platform === 'linux') {
      // Linux: ~/.1password/agent.sock
      socketPath = path.join(os.homedir(), '.1password', 'agent.sock');
    } else if (platform === 'win32') {
      // Windows: \\.\pipe\openssh-ssh-agent or named pipe
      // 1Password on Windows uses a named pipe, which is more complex
      // For now, return null and let system agent detection handle it
      return null;
    }

    if (socketPath && this.socketExists(socketPath)) {
      return {
        type: 'onepassword',
        socketPath
      };
    }

    return null;
  }

  /**
   * Detect YubiKey SSH agent
   */
  static detectYubiKey(): AgentInfo | null {
    const socketPath = path.join(os.homedir(), '.yubikey-agent.sock');

    if (this.socketExists(socketPath)) {
      return {
        type: 'yubikey',
        socketPath
      };
    }

    return null;
  }

  /**
   * Detect system SSH agent from SSH_AUTH_SOCK environment variable
   */
  static detectSystemAgent(): AgentInfo | null {
    const socketPath = process.env.SSH_AUTH_SOCK;

    if (socketPath && this.socketExists(socketPath)) {
      return {
        type: 'system',
        socketPath
      };
    }

    return null;
  }

  /**
   * Check if a socket file exists
   */
  private static socketExists(socketPath: string): boolean {
    try {
      const stat = fs.statSync(socketPath);
      return stat.isSocket();
    } catch {
      return false;
    }
  }

  /**
   * Get all available agents
   */
  static detectAll(): AgentInfo[] {
    const agents: AgentInfo[] = [];

    const onePassword = this.detect1Password();
    if (onePassword) agents.push(onePassword);

    const yubikey = this.detectYubiKey();
    if (yubikey) agents.push(yubikey);

    const system = this.detectSystemAgent();
    if (system) agents.push(system);

    return agents;
  }
}
