#include "ssh_session.h"
#include "ssh_channel.h"
#include "async_workers.h"
#include "utils.h"
#include <iostream>

namespace libssh_node {

Napi::Object SSHSession::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "SSHSession", {
    InstanceMethod("setOption", &SSHSession::SetOption),
    InstanceMethod("connect", &SSHSession::Connect),
    InstanceMethod("disconnect", &SSHSession::Disconnect),
    InstanceMethod("authenticatePassword", &SSHSession::AuthenticatePassword),
    InstanceMethod("authenticateAgent", &SSHSession::AuthenticateAgent),
    InstanceMethod("parseConfig", &SSHSession::ParseConfig),
    InstanceMethod("isConnected", &SSHSession::IsConnected),
    InstanceMethod("createChannel", &SSHSession::CreateChannel)
  });

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData(constructor);

  exports.Set("SSHSession", func);
  return exports;
}

SSHSession::SSHSession(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<SSHSession>(info), session_(nullptr), connected_(false) {
  Napi::Env env = info.Env();

  session_ = ssh_new();
  if (session_ == nullptr) {
    Napi::Error::New(env, "Failed to create SSH session").ThrowAsJavaScriptException();
    return;
  }

  // Set default options from constructor argument
  if (info.Length() > 0 && info[0].IsObject()) {
    Napi::Object options = info[0].As<Napi::Object>();

    std::string host = GetStringOption(options, "host");
    if (!host.empty()) {
      ssh_options_set(session_, SSH_OPTIONS_HOST, host.c_str());
    }

    int port = GetIntOption(options, "port", 22);
    ssh_options_set(session_, SSH_OPTIONS_PORT, &port);

    std::string user = GetStringOption(options, "user");
    if (!user.empty()) {
      ssh_options_set(session_, SSH_OPTIONS_USER, user.c_str());
    }

    std::string configFile = GetStringOption(options, "configFile");
    if (!configFile.empty()) {
      ssh_options_parse_config(session_, configFile.c_str());
    }

    std::string agentSocket = GetStringOption(options, "agentSocket");
    if (!agentSocket.empty()) {
      ssh_options_set(session_, SSH_OPTIONS_IDENTITY_AGENT, agentSocket.c_str());
    }

    int timeout = GetIntOption(options, "timeout", 0);
    if (timeout > 0) {
      long timeoutLong = timeout;
      ssh_options_set(session_, SSH_OPTIONS_TIMEOUT, &timeoutLong);
    }
  }
}

SSHSession::~SSHSession() {
  if (session_ != nullptr) {
    if (connected_) {
      ssh_disconnect(session_);
    }
    ssh_free(session_);
    session_ = nullptr;
  }
}

Napi::Value SSHSession::SetOption(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::Error::New(env, "Expected option name and value").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  std::string option = info[0].As<Napi::String>().Utf8Value();
  int result = SSH_ERROR;

  if (option == "host") {
    std::string value = info[1].As<Napi::String>().Utf8Value();
    result = ssh_options_set(session_, SSH_OPTIONS_HOST, value.c_str());
  } else if (option == "port") {
    int value = info[1].As<Napi::Number>().Int32Value();
    result = ssh_options_set(session_, SSH_OPTIONS_PORT, &value);
  } else if (option == "user") {
    std::string value = info[1].As<Napi::String>().Utf8Value();
    result = ssh_options_set(session_, SSH_OPTIONS_USER, value.c_str());
  } else if (option == "agentSocket") {
    std::string value = info[1].As<Napi::String>().Utf8Value();
    result = ssh_options_set(session_, SSH_OPTIONS_IDENTITY_AGENT, value.c_str());
  } else {
    Napi::Error::New(env, "Unknown option: " + option).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (result != SSH_OK) {
    CreateSSHError(env, session_, "Failed to set option").ThrowAsJavaScriptException();
  }

  return env.Undefined();
}

Napi::Value SSHSession::Connect(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  ConnectWorker* worker = new ConnectWorker(env, session_, deferred);
  worker->Queue();

  connected_ = true; // Will be set to false if connection fails

  return deferred.Promise();
}

Napi::Value SSHSession::Disconnect(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  DisconnectWorker* worker = new DisconnectWorker(env, session_, deferred);
  worker->Queue();

  connected_ = false;

  return deferred.Promise();
}

Napi::Value SSHSession::AuthenticatePassword(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::Error::New(env, "Expected username and password").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  std::string username = info[0].As<Napi::String>().Utf8Value();
  std::string password = info[1].As<Napi::String>().Utf8Value();

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  AuthPasswordWorker* worker = new AuthPasswordWorker(env, session_, username, password, deferred);
  worker->Queue();

  return deferred.Promise();
}

Napi::Value SSHSession::AuthenticateAgent(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  std::string username;
  if (info.Length() > 0 && info[0].IsString()) {
    username = info[0].As<Napi::String>().Utf8Value();
  }

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  AuthAgentWorker* worker = new AuthAgentWorker(env, session_, username, deferred);
  worker->Queue();

  return deferred.Promise();
}

Napi::Value SSHSession::ParseConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  const char* configFile = nullptr;
  if (info.Length() > 0 && info[0].IsString()) {
    std::string file = info[0].As<Napi::String>().Utf8Value();
    configFile = file.c_str();
  }

  int result = ssh_options_parse_config(session_, configFile);
  if (result != SSH_OK) {
    CreateSSHError(env, session_, "Failed to parse SSH config").ThrowAsJavaScriptException();
  }

  return env.Undefined();
}

Napi::Value SSHSession::IsConnected(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Boolean::New(env, connected_ && ssh_is_connected(session_));
}

Napi::Value SSHSession::CreateChannel(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!connected_) {
    Napi::Error::New(env, "Session is not connected").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::FunctionReference* channelConstructor = env.GetInstanceData<Napi::FunctionReference>();
  if (channelConstructor == nullptr) {
    Napi::Error::New(env, "Channel constructor not available").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  return SSHChannel::NewInstance(env, session_, Value());
}

} // namespace libssh_node
