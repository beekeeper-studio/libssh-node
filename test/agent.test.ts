import { AgentDetector } from '../lib/agent';

describe('AgentDetector', () => {
  describe('detect', () => {
    it('should return an agent or null', () => {
      const agent = AgentDetector.detect();
      if (agent !== null) {
        expect(agent).toHaveProperty('type');
        expect(agent).toHaveProperty('socketPath');
        expect(['onepassword', 'yubikey', 'system']).toContain(agent.type);
      }
    });
  });

  describe('detectAll', () => {
    it('should return an array of agents', () => {
      const agents = AgentDetector.detectAll();
      expect(Array.isArray(agents)).toBe(true);

      agents.forEach((agent) => {
        expect(agent).toHaveProperty('type');
        expect(agent).toHaveProperty('socketPath');
      });
    });
  });

  describe('detect1Password', () => {
    it('should detect 1Password agent if available', () => {
      const agent = AgentDetector.detect1Password();
      if (agent !== null) {
        expect(agent.type).toBe('onepassword');
        expect(agent.socketPath).toBeTruthy();
      }
    });
  });

  describe('detectYubiKey', () => {
    it('should detect YubiKey agent if available', () => {
      const agent = AgentDetector.detectYubiKey();
      if (agent !== null) {
        expect(agent.type).toBe('yubikey');
        expect(agent.socketPath).toBeTruthy();
      }
    });
  });

  describe('detectSystemAgent', () => {
    it('should detect system agent from SSH_AUTH_SOCK', () => {
      const agent = AgentDetector.detectSystemAgent();
      if (agent !== null) {
        expect(agent.type).toBe('system');
        expect(agent.socketPath).toBe(process.env.SSH_AUTH_SOCK);
      }
    });
  });
});
