import { SSHSession, AgentDetector } from '../lib';

async function main() {
  // Detect available agents
  console.log('Detecting SSH agents...');
  const agents = AgentDetector.detectAll();

  if (agents.length === 0) {
    console.log('No SSH agents found!');
    process.exit(1);
  }

  console.log(`Found ${agents.length} agent(s):`);
  agents.forEach((agent) => {
    console.log(`  - ${agent.type}: ${agent.socketPath}`);
  });

  // Use the first agent
  const agent = agents[0];
  console.log(`\nUsing ${agent.type} agent...`);

  const session = new SSHSession({
    host: 'example.com',
    port: 22,
    user: 'username',
    agentSocket: agent.socketPath
  });

  try {
    console.log('Connecting...');
    await session.connect();

    console.log('Authenticating with agent...');
    await session.authenticate({ useAgent: true });

    console.log('Success!');

    await session.disconnect();
  } catch (err) {
    console.error('Error:', err);
    process.exit(1);
  }
}

main();
