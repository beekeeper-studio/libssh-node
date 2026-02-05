#ifndef LIBSSH_NODE_ASYNC_WORKERS_H
#define LIBSSH_NODE_ASYNC_WORKERS_H

#include <napi.h>
#include <libssh/libssh.h>
#include <string>

namespace libssh_node {

// Base class for SSH async operations
class SSHAsyncWorker : public Napi::AsyncWorker {
public:
  SSHAsyncWorker(Napi::Env env, ssh_session session);
  virtual ~SSHAsyncWorker() = default;

protected:
  ssh_session session_;
  int result_;
  std::string errorMessage_;
};

// Connect operation
class ConnectWorker : public SSHAsyncWorker {
public:
  ConnectWorker(Napi::Env env, ssh_session session, const Napi::Promise::Deferred& deferred);
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error& error) override;

private:
  Napi::Promise::Deferred deferred_;
};

// Password authentication
class AuthPasswordWorker : public SSHAsyncWorker {
public:
  AuthPasswordWorker(Napi::Env env, ssh_session session,
                     const std::string& username, const std::string& password,
                     const Napi::Promise::Deferred& deferred);
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error& error) override;

private:
  std::string username_;
  std::string password_;
  Napi::Promise::Deferred deferred_;
};

// Agent authentication
class AuthAgentWorker : public SSHAsyncWorker {
public:
  AuthAgentWorker(Napi::Env env, ssh_session session,
                  const std::string& username,
                  const Napi::Promise::Deferred& deferred);
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error& error) override;

private:
  std::string username_;
  Napi::Promise::Deferred deferred_;
};

// Disconnect operation
class DisconnectWorker : public SSHAsyncWorker {
public:
  DisconnectWorker(Napi::Env env, ssh_session session, const Napi::Promise::Deferred& deferred);
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error& error) override;

private:
  Napi::Promise::Deferred deferred_;
};

} // namespace libssh_node

#endif // LIBSSH_NODE_ASYNC_WORKERS_H
