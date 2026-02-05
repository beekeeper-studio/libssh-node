#include "ssh_channel.h"
#include "utils.h"
#include <cstring>

namespace libssh_node {

// SSHChannel Implementation
Napi::Object SSHChannel::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "SSHChannel", {
    InstanceMethod("openSession", &SSHChannel::OpenSession),
    InstanceMethod("requestExec", &SSHChannel::RequestExec),
    InstanceMethod("requestForwardTcpIp", &SSHChannel::RequestForwardTcpIp),
    InstanceMethod("read", &SSHChannel::Read),
    InstanceMethod("write", &SSHChannel::Write),
    InstanceMethod("close", &SSHChannel::Close),
    InstanceMethod("isOpen", &SSHChannel::IsOpen)
  });

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData(constructor);

  exports.Set("SSHChannel", func);
  return exports;
}

Napi::Value SSHChannel::NewInstance(Napi::Env env, ssh_session session, Napi::Value sessionRef) {
  Napi::FunctionReference* constructor = env.GetInstanceData<Napi::FunctionReference>();
  Napi::Object obj = constructor->New({});

  SSHChannel* channel = SSHChannel::Unwrap(obj);
  channel->session_ = session;
  channel->sessionRef_ = Napi::Reference<Napi::Value>::New(sessionRef, 1);

  return obj;
}

SSHChannel::SSHChannel(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<SSHChannel>(info), session_(nullptr), channel_(nullptr), open_(false) {
  // Session will be set by NewInstance
}

SSHChannel::~SSHChannel() {
  if (channel_ != nullptr && open_) {
    ssh_channel_close(channel_);
    ssh_channel_free(channel_);
    channel_ = nullptr;
  }
  sessionRef_.Reset();
}

Napi::Value SSHChannel::OpenSession(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (channel_ != nullptr) {
    Napi::Error::New(env, "Channel already opened").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  channel_ = ssh_channel_new(session_);
  if (channel_ == nullptr) {
    Napi::Error::New(env, "Failed to create channel").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  ChannelOpenWorker* worker = new ChannelOpenWorker(env, channel_, deferred);
  worker->Queue();

  return deferred.Promise();
}

Napi::Value SSHChannel::RequestExec(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!open_) {
    Napi::Error::New(env, "Channel is not open").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::Error::New(env, "Expected command string").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  std::string command = info[0].As<Napi::String>().Utf8Value();

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  ChannelExecWorker* worker = new ChannelExecWorker(env, channel_, command, deferred);
  worker->Queue();

  return deferred.Promise();
}

Napi::Value SSHChannel::RequestForwardTcpIp(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::Error::New(env, "Expected remoteHost and remotePort").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  std::string remoteHost = info[0].As<Napi::String>().Utf8Value();
  int remotePort = info[1].As<Napi::Number>().Int32Value();

  std::string sourceHost = "127.0.0.1";
  int sourcePort = 0;

  if (info.Length() > 2 && info[2].IsString()) {
    sourceHost = info[2].As<Napi::String>().Utf8Value();
  }
  if (info.Length() > 3 && info[3].IsNumber()) {
    sourcePort = info[3].As<Napi::Number>().Int32Value();
  }

  if (channel_ == nullptr) {
    channel_ = ssh_channel_new(session_);
    if (channel_ == nullptr) {
      Napi::Error::New(env, "Failed to create channel").ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  ChannelForwardWorker* worker = new ChannelForwardWorker(
    env, channel_, remoteHost, remotePort, sourceHost, sourcePort, deferred);
  worker->Queue();

  return deferred.Promise();
}

Napi::Value SSHChannel::Read(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!open_) {
    Napi::Error::New(env, "Channel is not open").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  int maxBytes = 65536; // Default 64KB
  if (info.Length() > 0 && info[0].IsNumber()) {
    maxBytes = info[0].As<Napi::Number>().Int32Value();
  }

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  ChannelReadWorker* worker = new ChannelReadWorker(env, channel_, maxBytes, deferred);
  worker->Queue();

  return deferred.Promise();
}

Napi::Value SSHChannel::Write(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!open_) {
    Napi::Error::New(env, "Channel is not open").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (info.Length() < 1 || !info[0].IsBuffer()) {
    Napi::Error::New(env, "Expected Buffer").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::Buffer<char> buffer = info[0].As<Napi::Buffer<char>>();
  std::vector<char> data(buffer.Data(), buffer.Data() + buffer.Length());

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  ChannelWriteWorker* worker = new ChannelWriteWorker(env, channel_, data, deferred);
  worker->Queue();

  return deferred.Promise();
}

Napi::Value SSHChannel::Close(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!open_) {
    return env.Undefined();
  }

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  ChannelCloseWorker* worker = new ChannelCloseWorker(env, channel_, deferred);
  worker->Queue();

  open_ = false;

  return deferred.Promise();
}

Napi::Value SSHChannel::IsOpen(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Boolean::New(env, open_ && ssh_channel_is_open(channel_));
}

// Async Workers Implementation

// ChannelOpenWorker
ChannelOpenWorker::ChannelOpenWorker(Napi::Env env, ssh_channel channel, const Napi::Promise::Deferred& deferred)
    : Napi::AsyncWorker(env), channel_(channel), deferred_(deferred), result_(SSH_ERROR) {}

void ChannelOpenWorker::Execute() {
  result_ = ssh_channel_open_session(channel_);
  if (result_ != SSH_OK) {
    errorMessage_ = "Failed to open channel session";
  }
}

void ChannelOpenWorker::OnOK() {
  if (result_ == SSH_OK) {
    SSHChannel* channelObj = static_cast<SSHChannel*>(deferred_.Env().GetInstanceData<void>());
    if (channelObj) {
      channelObj->open_ = true;
    }
    deferred_.Resolve(Env().Undefined());
  } else {
    deferred_.Reject(Napi::Error::New(Env(), errorMessage_).Value());
  }
}

void ChannelOpenWorker::OnError(const Napi::Error& error) {
  deferred_.Reject(error.Value());
}

// ChannelForwardWorker
ChannelForwardWorker::ChannelForwardWorker(Napi::Env env, ssh_channel channel,
                                           const std::string& remoteHost, int remotePort,
                                           const std::string& sourceHost, int sourcePort,
                                           const Napi::Promise::Deferred& deferred)
    : Napi::AsyncWorker(env), channel_(channel), remoteHost_(remoteHost), remotePort_(remotePort),
      sourceHost_(sourceHost), sourcePort_(sourcePort), deferred_(deferred), result_(SSH_ERROR) {}

void ChannelForwardWorker::Execute() {
  result_ = ssh_channel_open_forward(channel_,
                                      remoteHost_.c_str(), remotePort_,
                                      sourceHost_.c_str(), sourcePort_);
  if (result_ != SSH_OK) {
    errorMessage_ = "Failed to open forward channel";
  }
}

void ChannelForwardWorker::OnOK() {
  if (result_ == SSH_OK) {
    deferred_.Resolve(Env().Undefined());
  } else {
    deferred_.Reject(Napi::Error::New(Env(), errorMessage_).Value());
  }
}

void ChannelForwardWorker::OnError(const Napi::Error& error) {
  deferred_.Reject(error.Value());
}

// ChannelReadWorker
ChannelReadWorker::ChannelReadWorker(Napi::Env env, ssh_channel channel, int maxBytes,
                                     const Napi::Promise::Deferred& deferred)
    : Napi::AsyncWorker(env), channel_(channel), maxBytes_(maxBytes),
      deferred_(deferred), bytesRead_(0) {
  buffer_.resize(maxBytes);
}

void ChannelReadWorker::Execute() {
  bytesRead_ = ssh_channel_read(channel_, buffer_.data(), maxBytes_, 0);
  if (bytesRead_ < 0) {
    errorMessage_ = "Failed to read from channel";
  }
}

void ChannelReadWorker::OnOK() {
  if (bytesRead_ >= 0) {
    Napi::Buffer<char> result = Napi::Buffer<char>::Copy(Env(), buffer_.data(), bytesRead_);
    deferred_.Resolve(result);
  } else {
    deferred_.Reject(Napi::Error::New(Env(), errorMessage_).Value());
  }
}

void ChannelReadWorker::OnError(const Napi::Error& error) {
  deferred_.Reject(error.Value());
}

// ChannelWriteWorker
ChannelWriteWorker::ChannelWriteWorker(Napi::Env env, ssh_channel channel,
                                       const std::vector<char>& data,
                                       const Napi::Promise::Deferred& deferred)
    : Napi::AsyncWorker(env), channel_(channel), data_(data),
      deferred_(deferred), bytesWritten_(0) {}

void ChannelWriteWorker::Execute() {
  bytesWritten_ = ssh_channel_write(channel_, data_.data(), data_.size());
  if (bytesWritten_ < 0) {
    errorMessage_ = "Failed to write to channel";
  }
}

void ChannelWriteWorker::OnOK() {
  if (bytesWritten_ >= 0) {
    deferred_.Resolve(Napi::Number::New(Env(), bytesWritten_));
  } else {
    deferred_.Reject(Napi::Error::New(Env(), errorMessage_).Value());
  }
}

void ChannelWriteWorker::OnError(const Napi::Error& error) {
  deferred_.Reject(error.Value());
}

// ChannelExecWorker
ChannelExecWorker::ChannelExecWorker(Napi::Env env, ssh_channel channel,
                                     const std::string& command,
                                     const Napi::Promise::Deferred& deferred)
    : Napi::AsyncWorker(env), channel_(channel), command_(command),
      deferred_(deferred), result_(SSH_ERROR) {}

void ChannelExecWorker::Execute() {
  result_ = ssh_channel_request_exec(channel_, command_.c_str());
  if (result_ != SSH_OK) {
    errorMessage_ = "Failed to execute command";
  }
}

void ChannelExecWorker::OnOK() {
  if (result_ == SSH_OK) {
    deferred_.Resolve(Env().Undefined());
  } else {
    deferred_.Reject(Napi::Error::New(Env(), errorMessage_).Value());
  }
}

void ChannelExecWorker::OnError(const Napi::Error& error) {
  deferred_.Reject(error.Value());
}

// ChannelCloseWorker
ChannelCloseWorker::ChannelCloseWorker(Napi::Env env, ssh_channel channel, const Napi::Promise::Deferred& deferred)
    : Napi::AsyncWorker(env), channel_(channel), deferred_(deferred) {}

void ChannelCloseWorker::Execute() {
  ssh_channel_send_eof(channel_);
  ssh_channel_close(channel_);
}

void ChannelCloseWorker::OnOK() {
  deferred_.Resolve(Env().Undefined());
}

void ChannelCloseWorker::OnError(const Napi::Error& error) {
  deferred_.Reject(error.Value());
}

} // namespace libssh_node
