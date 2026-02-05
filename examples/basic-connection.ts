import { SSHSession } from '../lib';

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
    console.log('Connected!');

    console.log('Authenticating with agent...');
    await session.authenticate({ useAgent: true });
    console.log('Authenticated!');

    console.log('Disconnecting...');
    await session.disconnect();
    console.log('Disconnected!');
  } catch (err) {
    console.error('Error:', err);
    process.exit(1);
  }
}

main();
