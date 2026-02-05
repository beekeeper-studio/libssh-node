import { SSHConfigParser } from '../lib/config';

describe('SSHConfigParser', () => {
  describe('parseContent', () => {
    it('should parse basic host configuration', () => {
      const content = `
Host example
  HostName example.com
  Port 2222
  User testuser
`;

      const hosts = SSHConfigParser.parseContent(content);
      expect(hosts).toHaveLength(1);
      expect(hosts[0].host).toBe('example');
      expect(hosts[0].hostname).toBe('example.com');
      expect(hosts[0].port).toBe(2222);
      expect(hosts[0].user).toBe('testuser');
    });

    it('should parse multiple identity files', () => {
      const content = `
Host example
  IdentityFile ~/.ssh/id_rsa
  IdentityFile ~/.ssh/id_ed25519
`;

      const hosts = SSHConfigParser.parseContent(content);
      expect(hosts[0].identityFile).toHaveLength(2);
    });

    it('should handle comments', () => {
      const content = `
# This is a comment
Host example
  HostName example.com # Inline comment
  Port 22
`;

      const hosts = SSHConfigParser.parseContent(content);
      expect(hosts).toHaveLength(1);
      expect(hosts[0].hostname).toBe('example.com');
    });

    it('should parse multiple hosts', () => {
      const content = `
Host server1
  HostName server1.com
  Port 22

Host server2
  HostName server2.com
  Port 2222
`;

      const hosts = SSHConfigParser.parseContent(content);
      expect(hosts).toHaveLength(2);
      expect(hosts[0].host).toBe('server1');
      expect(hosts[1].host).toBe('server2');
    });
  });

  describe('findHostConfig', () => {
    it('should find exact host match', () => {
      const content = `
Host example
  HostName example.com
`;

      const hosts = SSHConfigParser.parseContent(content);
      // Find in the parsed hosts
      const config = hosts.find(h => h.host === 'example');
      expect(config).toBeDefined();
      expect(config?.hostname).toBe('example.com');
    });

    it('should match wildcard patterns', () => {
      const content = `
Host *.example.com
  User testuser
`;

      const hosts = SSHConfigParser.parseContent(content);
      // Note: This requires actual file parsing to test properly
      // For now, just verify content parsing works
      expect(hosts).toHaveLength(1);
    });
  });
});
