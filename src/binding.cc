#include <napi.h>
#include "ssh_session.h"
#include "ssh_channel.h"
#include "ssh_sftp.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  libssh_node::SSHSession::Init(env, exports);
  libssh_node::SSHChannel::Init(env, exports);
  libssh_node::SSHSftp::Init(env, exports);

  return exports;
}

NODE_API_MODULE(libssh_node, Init)
