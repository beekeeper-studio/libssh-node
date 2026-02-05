export class SSHError extends Error {
  constructor(message: string) {
    super(message);
    this.name = 'SSHError';
  }
}

export class SSHConnectionError extends SSHError {
  constructor(message: string) {
    super(message);
    this.name = 'SSHConnectionError';
  }
}

export class SSHAuthenticationError extends SSHError {
  constructor(message: string) {
    super(message);
    this.name = 'SSHAuthenticationError';
  }
}

export class SSHChannelError extends SSHError {
  constructor(message: string) {
    super(message);
    this.name = 'SSHChannelError';
  }
}

export class SSHTunnelError extends SSHError {
  constructor(message: string) {
    super(message);
    this.name = 'SSHTunnelError';
  }
}
