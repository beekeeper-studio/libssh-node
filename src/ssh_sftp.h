#ifndef LIBSSH_NODE_SSH_SFTP_H
#define LIBSSH_NODE_SSH_SFTP_H

#include <napi.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

namespace libssh_node {

// Placeholder for Phase 5
class SSHSftp : public Napi::ObjectWrap<SSHSftp> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  explicit SSHSftp(const Napi::CallbackInfo& info);
  ~SSHSftp();

private:
  sftp_session sftp_;
};

} // namespace libssh_node

#endif // LIBSSH_NODE_SSH_SFTP_H
