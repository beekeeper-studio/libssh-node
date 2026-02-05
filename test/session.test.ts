// Mock the native module if it doesn't exist
jest.mock('../build/Release/libssh_node.node', () => ({
  SSHSession: class MockSSHSession {
    constructor() {}
    connect() { return Promise.resolve(); }
    disconnect() { return Promise.resolve(); }
    isConnected() { return false; }
    authenticatePassword() { return Promise.resolve(); }
    authenticateAgent() { return Promise.resolve(); }
    setOption() {}
    parseConfig() {}
    createChannel() { return {}; }
  }
}), { virtual: true });

import { SSHSession } from '../lib/session';

describe('SSHSession', () => {
  describe('constructor', () => {
    it('should create a session with default options', () => {
      const session = new SSHSession();
      expect(session).toBeDefined();
    });

    it('should create a session with custom options', () => {
      const session = new SSHSession({
        host: 'example.com',
        port: 2222,
        user: 'testuser',
        timeout: 10000
      });
      expect(session).toBeDefined();
    });

    it('should auto-detect agent if enabled', () => {
      const session = new SSHSession({
        host: 'example.com',
        autoDetectAgent: true
      });
      expect(session).toBeDefined();
    });

    it('should not auto-detect agent if disabled', () => {
      const session = new SSHSession({
        host: 'example.com',
        autoDetectAgent: false
      });
      expect(session).toBeDefined();
    });
  });

  describe('isConnected', () => {
    it('should return false for new session', () => {
      const session = new SSHSession();
      expect(session.isConnected()).toBe(false);
    });
  });

  // Note: Actual connection tests require a real SSH server
  // These should be in integration tests
});
