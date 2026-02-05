# TODO List

## Critical (Before First Use)

### Build & Compilation
- [ ] Install libssh on development machine
- [ ] Run `yarn install`
- [ ] Build native module (`yarn build:native`)
- [ ] Fix any compilation errors
- [ ] Build TypeScript (`yarn build:ts`)
- [ ] Verify build output in `build/Release/libssh_node.node`

### Testing
- [ ] Set up test SSH server (local or Docker)
- [ ] Test basic connection
- [ ] Test password authentication
- [ ] Test agent authentication
- [ ] Test channel creation
- [ ] Test tunnel creation
- [ ] Test data flow through tunnel
- [ ] Run unit tests (`yarn test`)

### Bug Fixes
- [ ] Fix any runtime errors discovered during testing
- [ ] Ensure proper memory cleanup (no leaks)
- [ ] Test error handling paths
- [ ] Verify async operations complete properly

### Documentation
- [ ] Verify all examples work
- [ ] Update documentation if APIs changed during testing
- [ ] Add troubleshooting section for discovered issues

## High Priority (Before Production)

### Platform Testing
- [ ] Test on Linux (Ubuntu 20.04, 22.04)
- [ ] Test on macOS (12+, both Intel and Apple Silicon)
- [ ] Test on Windows 10/11
- [ ] Test with Electron 31

### Integration Testing
- [ ] Test with MySQL database
- [ ] Test with PostgreSQL database
- [ ] Test with MongoDB
- [ ] Test with Redis
- [ ] Test with real 1Password agent
- [ ] Test with real YubiKey agent
- [ ] Test SSH config file integration
- [ ] Test multiple simultaneous tunnels

### Performance
- [ ] Profile tunnel throughput
- [ ] Test with large data transfers
- [ ] Test connection stability over time
- [ ] Memory leak testing with Valgrind (Linux)
- [ ] Stress test with many concurrent connections

### Error Handling
- [ ] Test connection timeout scenarios
- [ ] Test authentication failures
- [ ] Test network interruption recovery
- [ ] Test invalid configuration handling
- [ ] Improve error messages based on testing

### Code Quality
- [ ] Run ESLint and fix issues
- [ ] Add inline code comments
- [ ] Review C++ code for edge cases
- [ ] Review TypeScript code for type safety
- [ ] Add debug logging option

## Medium Priority (Nice to Have)

### Features
- [ ] Implement connection keep-alive
- [ ] Add connection recovery/retry logic
- [ ] Add event emitters for status changes
- [ ] Add connection metrics/statistics
- [ ] Add debug logging mode
- [ ] Support for SSH compression option
- [ ] Support for ProxyJump in config files

### Testing
- [ ] Increase test coverage to >80%
- [ ] Add integration test suite
- [ ] Add benchmark tests
- [ ] Set up CI/CD pipeline (GitHub Actions)
- [ ] Automated testing on multiple platforms

### Documentation
- [ ] Add API reference documentation (TypeDoc)
- [ ] Add architecture documentation
- [ ] Add contribution guidelines
- [ ] Add changelog
- [ ] Add migration guide (from other SSH libs)
- [ ] Record demo video

### Developer Experience
- [ ] Add TypeScript strict mode
- [ ] Improve error messages with suggestions
- [ ] Add more examples
- [ ] Create interactive documentation
- [ ] Add debugging guide

## Low Priority (Future)

### Phase 5: SFTP Implementation
- [ ] Implement SSHSftp class
- [ ] Add file read/write operations
- [ ] Add directory operations
- [ ] Add progress callbacks
- [ ] Test SFTP operations
- [ ] Document SFTP usage

### Advanced Features
- [ ] SSH tunneling server mode (reverse tunnels)
- [ ] Dynamic port forwarding (SOCKS proxy)
- [ ] X11 forwarding support
- [ ] SSH command execution with streaming output
- [ ] SCP file transfer
- [ ] Known hosts file support
- [ ] Host key verification options

### Optimization
- [ ] Connection pooling
- [ ] Tunnel connection reuse
- [ ] Buffer size optimization
- [ ] Zero-copy data forwarding
- [ ] Parallel tunnel processing

### Platform Support
- [ ] ARM64 Linux testing
- [ ] FreeBSD support
- [ ] Alpine Linux support
- [ ] Older Node.js versions (14.x, 16.x)

### Distribution
- [ ] Publish to npm registry
- [ ] Create prebuilt binaries (node-pre-gyp)
- [ ] Set up binary hosting
- [ ] Create homebrew formula
- [ ] Create Windows installer

## Completed ✓

- ✓ Project structure
- ✓ C++ native implementation
  - ✓ SSHSession class
  - ✓ SSHChannel class
  - ✓ Async workers
  - ✓ Error handling
- ✓ TypeScript wrapper
  - ✓ Session wrapper
  - ✓ Channel wrapper
  - ✓ Tunnel class
  - ✓ Agent detection
  - ✓ Config parser
  - ✓ Error classes
- ✓ Documentation
  - ✓ README
  - ✓ Tunneling guide
  - ✓ Agents guide
  - ✓ Building guide
- ✓ Examples
  - ✓ Basic connection
  - ✓ Password auth
  - ✓ Agent auth
  - ✓ SSH tunnel
- ✓ Test structure
  - ✓ Agent tests
  - ✓ Config tests
  - ✓ Session tests
- ✓ Build configuration
  - ✓ binding.gyp
  - ✓ package.json
  - ✓ tsconfig.json
  - ✓ jest.config.js

## Notes

### Known Issues to Fix
1. SSHChannel reference management needs verification
2. Windows-specific agent paths may need adjustment
3. Thread safety documentation vs enforcement
4. Some error messages could be more descriptive

### Testing Strategy
1. Start with local SSH server (Docker recommended)
2. Test basic operations first
3. Then test tunneling
4. Then test with real databases
5. Finally test with agents and config files

### Release Checklist
- [ ] All critical tests passing
- [ ] Builds on Linux, macOS, Windows
- [ ] Works with Electron 31
- [ ] Documentation complete
- [ ] Examples verified
- [ ] Version number set
- [ ] License file added
- [ ] Contributing guidelines added
- [ ] Changelog started

### First Release Scope
For v0.1.0, include:
- Basic SSH connections
- Password and agent authentication
- Port forwarding/tunneling
- Agent auto-detection
- SSH config parsing
- TypeScript types
- Basic documentation
- Working examples

Exclude (for later):
- SFTP support
- Advanced features
- Optimizations
- Extensive testing
