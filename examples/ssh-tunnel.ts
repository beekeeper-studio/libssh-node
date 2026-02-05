import { SSHSession, SSHTunnel } from '../lib';

async function main() {
  const session = new SSHSession({
    host: 'example.com',
    port: 22,
    user: 'username',
    autoDetectAgent: true
  });

  try {
    console.log('Connecting to SSH server...');
    await session.connect();

    console.log('Authenticating...');
    await session.authenticate({ useAgent: true });

    console.log('Creating tunnel...');
    const tunnel = new SSHTunnel({
      session,
      localHost: '127.0.0.1',
      localPort: 3307, // Local MySQL port
      remoteHost: 'localhost',
      remotePort: 3306 // Remote MySQL port
    });

    await tunnel.start();

    const address = tunnel.getLocalAddress();
    console.log(`Tunnel started! Connect to localhost:${address?.port}`);
    console.log('Press Ctrl+C to stop...');

    // Keep running until interrupted
    process.on('SIGINT', async () => {
      console.log('\nStopping tunnel...');
      await tunnel.stop();
      await session.disconnect();
      console.log('Stopped!');
      process.exit(0);
    });

  } catch (err) {
    console.error('Error:', err);
    await session.disconnect();
    process.exit(1);
  }
}

main();
