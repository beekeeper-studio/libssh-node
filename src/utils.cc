#include "utils.h"
#include <sstream>

namespace libssh_node {

Napi::Error CreateSSHError(Napi::Env env, ssh_session session) {
  std::string errorMsg = "SSH Error";
  if (session != nullptr) {
    const char* error = ssh_get_error(session);
    if (error != nullptr) {
      errorMsg = error;
    }
  }
  return Napi::Error::New(env, errorMsg);
}

Napi::Error CreateSSHError(Napi::Env env, ssh_session session, const std::string& message) {
  std::ostringstream oss;
  oss << message;

  if (session != nullptr) {
    const char* error = ssh_get_error(session);
    if (error != nullptr) {
      oss << ": " << error;
    }
  }

  return Napi::Error::New(env, oss.str());
}

std::string GetStringOption(const Napi::Object& obj, const char* key, const std::string& defaultValue) {
  if (obj.Has(key)) {
    Napi::Value val = obj.Get(key);
    if (val.IsString()) {
      return val.As<Napi::String>().Utf8Value();
    }
  }
  return defaultValue;
}

int GetIntOption(const Napi::Object& obj, const char* key, int defaultValue) {
  if (obj.Has(key)) {
    Napi::Value val = obj.Get(key);
    if (val.IsNumber()) {
      return val.As<Napi::Number>().Int32Value();
    }
  }
  return defaultValue;
}

bool GetBoolOption(const Napi::Object& obj, const char* key, bool defaultValue) {
  if (obj.Has(key)) {
    Napi::Value val = obj.Get(key);
    if (val.IsBoolean()) {
      return val.As<Napi::Boolean>().Value();
    }
  }
  return defaultValue;
}

} // namespace libssh_node
