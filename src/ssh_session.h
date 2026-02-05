#ifndef LIBSSH_NODE_SSH_SESSION_H
#define LIBSSH_NODE_SSH_SESSION_H

#include <napi.h>
#include <libssh/libssh.h>
#include <mutex>

namespace libssh_node {

class SSHSession : public Napi::ObjectWrap<SSHSession> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  explicit SSHSession(const Napi::CallbackInfo& info);
  ~SSHSession();

private:
  // Session methods
  Napi::Value SetOption(const Napi::CallbackInfo& info);
  Napi::Value Connect(const Napi::CallbackInfo& info);
  Napi::Value Disconnect(const Napi::CallbackInfo& info);
  Napi::Value AuthenticatePassword(const Napi::CallbackInfo& info);
  Napi::Value AuthenticateAgent(const Napi::CallbackInfo& info);
  Napi::Value ParseConfig(const Napi::CallbackInfo& info);
  Napi::Value IsConnected(const Napi::CallbackInfo& info);
  Napi::Value CreateChannel(const Napi::CallbackInfo& info);

  ssh_session session_;
  std::mutex mutex_;
  bool connected_;

  friend class SSHChannel;
};

} // namespace libssh_node

#endif // LIBSSH_NODE_SSH_SESSION_H
