import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';

export interface SSHConfigHost {
  host: string;
  hostname?: string;
  port?: number;
  user?: string;
  identityFile?: string[];
  proxyJump?: string;
  forwardAgent?: boolean;
  [key: string]: string | number | boolean | string[] | undefined;
}

export class SSHConfigParser {
  /**
   * Parse SSH config file (default: ~/.ssh/config)
   */
  static parse(configPath?: string): SSHConfigHost[] {
    if (!configPath) {
      configPath = path.join(os.homedir(), '.ssh', 'config');
    }

    if (!fs.existsSync(configPath)) {
      return [];
    }

    const content = fs.readFileSync(configPath, 'utf-8');
    return this.parseContent(content);
  }

  /**
   * Parse SSH config file content
   */
  static parseContent(content: string): SSHConfigHost[] {
    const hosts: SSHConfigHost[] = [];
    let currentHost: SSHConfigHost | null = null;

    const lines = content.split('\n');

    for (let line of lines) {
      // Remove comments
      const commentIndex = line.indexOf('#');
      if (commentIndex !== -1) {
        line = line.substring(0, commentIndex);
      }

      line = line.trim();
      if (!line) continue;

      const parts = line.split(/\s+/);
      const key = parts[0].toLowerCase();
      const value = parts.slice(1).join(' ');

      if (key === 'host') {
        if (currentHost) {
          hosts.push(currentHost);
        }
        currentHost = { host: value };
      } else if (currentHost) {
        this.parseOption(currentHost, key, value);
      }
    }

    if (currentHost) {
      hosts.push(currentHost);
    }

    return hosts;
  }

  /**
   * Find host configuration by hostname
   */
  static findHostConfig(hostname: string, configPath?: string): SSHConfigHost | null {
    const hosts = this.parse(configPath);

    for (const host of hosts) {
      if (this.matchHost(hostname, host.host)) {
        return host;
      }
    }

    return null;
  }

  /**
   * Match hostname against host pattern (supports wildcards)
   */
  private static matchHost(hostname: string, pattern: string): boolean {
    if (pattern === '*') return true;
    if (pattern === hostname) return true;

    // Convert SSH pattern to regex
    const regexPattern = pattern
      .replace(/\./g, '\\.')
      .replace(/\*/g, '.*')
      .replace(/\?/g, '.');

    const regex = new RegExp(`^${regexPattern}$`, 'i');
    return regex.test(hostname);
  }

  /**
   * Parse individual config option
   */
  private static parseOption(host: SSHConfigHost, key: string, value: string): void {
    switch (key) {
      case 'hostname':
        host.hostname = value;
        break;
      case 'port':
        host.port = parseInt(value, 10);
        break;
      case 'user':
        host.user = value;
        break;
      case 'identityfile':
        if (!host.identityFile) {
          host.identityFile = [];
        }
        // Expand ~ to home directory
        const expandedValue = value.startsWith('~')
          ? path.join(os.homedir(), value.slice(1))
          : value;
        host.identityFile.push(expandedValue);
        break;
      case 'proxyjump':
        host.proxyJump = value;
        break;
      case 'forwardagent':
        host.forwardAgent = value.toLowerCase() === 'yes';
        break;
      default:
        // Store other options as-is
        host[key] = value;
        break;
    }
  }

  /**
   * Merge host config with default values
   */
  static mergeConfig(baseConfig: Partial<SSHConfigHost>, hostConfig: SSHConfigHost): SSHConfigHost {
    return {
      ...baseConfig,
      ...hostConfig
    };
  }
}
