#include "async_workers.h"
#include "utils.h"

namespace libssh_node {

// Base SSHAsyncWorker
SSHAsyncWorker::SSHAsyncWorker(Napi::Env env, ssh_session session)
    : Napi::AsyncWorker(env), session_(session), result_(SSH_ERROR) {}

// ConnectWorker
ConnectWorker::ConnectWorker(Napi::Env env, ssh_session session, const Napi::Promise::Deferred& deferred)
    : SSHAsyncWorker(env, session), deferred_(deferred) {}

void ConnectWorker::Execute() {
  result_ = ssh_connect(session_);
  if (result_ != SSH_OK) {
    const char* error = ssh_get_error(session_);
    errorMessage_ = error ? error : "Connection failed";
  }
}

void ConnectWorker::OnOK() {
  deferred_.Resolve(Env().Undefined());
}

void ConnectWorker::OnError(const Napi::Error& error) {
  deferred_.Reject(error.Value());
}

// AuthPasswordWorker
AuthPasswordWorker::AuthPasswordWorker(Napi::Env env, ssh_session session,
                                       const std::string& username, const std::string& password,
                                       const Napi::Promise::Deferred& deferred)
    : SSHAsyncWorker(env, session), username_(username), password_(password), deferred_(deferred) {}

void AuthPasswordWorker::Execute() {
  result_ = ssh_userauth_password(session_, username_.empty() ? nullptr : username_.c_str(), password_.c_str());
  if (result_ != SSH_AUTH_SUCCESS) {
    const char* error = ssh_get_error(session_);
    errorMessage_ = error ? error : "Authentication failed";
  }
}

void AuthPasswordWorker::OnOK() {
  if (result_ == SSH_AUTH_SUCCESS) {
    deferred_.Resolve(Env().Undefined());
  } else {
    deferred_.Reject(Napi::Error::New(Env(), errorMessage_).Value());
  }
}

void AuthPasswordWorker::OnError(const Napi::Error& error) {
  deferred_.Reject(error.Value());
}

// AuthAgentWorker
AuthAgentWorker::AuthAgentWorker(Napi::Env env, ssh_session session,
                                 const std::string& username,
                                 const Napi::Promise::Deferred& deferred)
    : SSHAsyncWorker(env, session), username_(username), deferred_(deferred) {}

void AuthAgentWorker::Execute() {
  result_ = ssh_userauth_agent(session_, username_.empty() ? nullptr : username_.c_str());
  if (result_ != SSH_AUTH_SUCCESS) {
    const char* error = ssh_get_error(session_);
    errorMessage_ = error ? error : "Agent authentication failed";
  }
}

void AuthAgentWorker::OnOK() {
  if (result_ == SSH_AUTH_SUCCESS) {
    deferred_.Resolve(Env().Undefined());
  } else {
    deferred_.Reject(Napi::Error::New(Env(), errorMessage_).Value());
  }
}

void AuthAgentWorker::OnError(const Napi::Error& error) {
  deferred_.Reject(error.Value());
}

// DisconnectWorker
DisconnectWorker::DisconnectWorker(Napi::Env env, ssh_session session, const Napi::Promise::Deferred& deferred)
    : SSHAsyncWorker(env, session), deferred_(deferred) {}

void DisconnectWorker::Execute() {
  ssh_disconnect(session_);
  result_ = SSH_OK;
}

void DisconnectWorker::OnOK() {
  deferred_.Resolve(Env().Undefined());
}

void DisconnectWorker::OnError(const Napi::Error& error) {
  deferred_.Reject(error.Value());
}

} // namespace libssh_node
