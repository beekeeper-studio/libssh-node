import { SSHSession } from '../lib';

async function main() {
  const session = new SSHSession({
    host: 'example.com',
    port: 22,
    user: 'username'
  });

  try {
    console.log('Connecting...');
    await session.connect();

    console.log('Authenticating with password...');
    await session.authenticate({
      username: 'username',
      password: 'your-password-here'
    });

    console.log('Success!');

    await session.disconnect();
  } catch (err) {
    console.error('Error:', err);
    process.exit(1);
  }
}

main();
