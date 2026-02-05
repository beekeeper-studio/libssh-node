#ifndef LIBSSH_NODE_SSH_CHANNEL_H
#define LIBSSH_NODE_SSH_CHANNEL_H

#include <napi.h>
#include <libssh/libssh.h>
#include <mutex>

namespace libssh_node {

// Forward declarations for async workers
class ChannelOpenWorker;
class ChannelForwardWorker;
class ChannelReadWorker;
class ChannelWriteWorker;
class ChannelCloseWorker;
class ChannelExecWorker;

class SSHChannel : public Napi::ObjectWrap<SSHChannel> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Value NewInstance(Napi::Env env, ssh_session session, Napi::Value sessionRef);

  explicit SSHChannel(const Napi::CallbackInfo& info);
  ~SSHChannel();

private:
  // Channel methods
  Napi::Value OpenSession(const Napi::CallbackInfo& info);
  Napi::Value RequestExec(const Napi::CallbackInfo& info);
  Napi::Value RequestForwardTcpIp(const Napi::CallbackInfo& info);
  Napi::Value Read(const Napi::CallbackInfo& info);
  Napi::Value Write(const Napi::CallbackInfo& info);
  Napi::Value Close(const Napi::CallbackInfo& info);
  Napi::Value IsOpen(const Napi::CallbackInfo& info);

  ssh_session session_;
  ssh_channel channel_;
  std::mutex mutex_;
  bool open_;
  Napi::Reference<Napi::Value> sessionRef_; // Keep session alive

  friend class ChannelOpenWorker;
  friend class ChannelForwardWorker;
  friend class ChannelReadWorker;
  friend class ChannelWriteWorker;
  friend class ChannelCloseWorker;
  friend class ChannelExecWorker;
};

// Async workers for channel operations
class ChannelOpenWorker : public Napi::AsyncWorker {
public:
  ChannelOpenWorker(Napi::Env env, ssh_channel channel, const Napi::Promise::Deferred& deferred);
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error& error) override;

private:
  ssh_channel channel_;
  Napi::Promise::Deferred deferred_;
  int result_;
  std::string errorMessage_;
};

class ChannelForwardWorker : public Napi::AsyncWorker {
public:
  ChannelForwardWorker(Napi::Env env, ssh_channel channel,
                       const std::string& remoteHost, int remotePort,
                       const std::string& sourceHost, int sourcePort,
                       const Napi::Promise::Deferred& deferred);
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error& error) override;

private:
  ssh_channel channel_;
  std::string remoteHost_;
  int remotePort_;
  std::string sourceHost_;
  int sourcePort_;
  Napi::Promise::Deferred deferred_;
  int result_;
  std::string errorMessage_;
};

class ChannelReadWorker : public Napi::AsyncWorker {
public:
  ChannelReadWorker(Napi::Env env, ssh_channel channel, int maxBytes,
                    const Napi::Promise::Deferred& deferred);
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error& error) override;

private:
  ssh_channel channel_;
  int maxBytes_;
  Napi::Promise::Deferred deferred_;
  std::vector<char> buffer_;
  int bytesRead_;
  std::string errorMessage_;
};

class ChannelWriteWorker : public Napi::AsyncWorker {
public:
  ChannelWriteWorker(Napi::Env env, ssh_channel channel,
                     const std::vector<char>& data,
                     const Napi::Promise::Deferred& deferred);
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error& error) override;

private:
  ssh_channel channel_;
  std::vector<char> data_;
  Napi::Promise::Deferred deferred_;
  int bytesWritten_;
  std::string errorMessage_;
};

class ChannelExecWorker : public Napi::AsyncWorker {
public:
  ChannelExecWorker(Napi::Env env, ssh_channel channel,
                    const std::string& command,
                    const Napi::Promise::Deferred& deferred);
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error& error) override;

private:
  ssh_channel channel_;
  std::string command_;
  Napi::Promise::Deferred deferred_;
  int result_;
  std::string errorMessage_;
};

class ChannelCloseWorker : public Napi::AsyncWorker {
public:
  ChannelCloseWorker(Napi::Env env, ssh_channel channel, const Napi::Promise::Deferred& deferred);
  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error& error) override;

private:
  ssh_channel channel_;
  Napi::Promise::Deferred deferred_;
};

} // namespace libssh_node

#endif // LIBSSH_NODE_SSH_CHANNEL_H
