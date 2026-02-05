#include "ssh_sftp.h"

namespace libssh_node {

Napi::Object SSHSftp::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "SSHSftp", {});

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);

  exports.Set("SSHSftp", func);
  return exports;
}

SSHSftp::SSHSftp(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<SSHSftp>(info), sftp_(nullptr) {
  // To be implemented in Phase 5
}

SSHSftp::~SSHSftp() {
  if (sftp_ != nullptr) {
    sftp_free(sftp_);
    sftp_ = nullptr;
  }
}

} // namespace libssh_node
