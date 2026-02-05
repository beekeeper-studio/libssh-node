#ifndef LIBSSH_NODE_UTILS_H
#define LIBSSH_NODE_UTILS_H

#include <napi.h>
#include <libssh/libssh.h>
#include <string>

namespace libssh_node {

// Error handling utilities
Napi::Error CreateSSHError(Napi::Env env, ssh_session session);
Napi::Error CreateSSHError(Napi::Env env, ssh_session session, const std::string& message);

// Type conversion utilities
std::string GetStringOption(const Napi::Object& obj, const char* key, const std::string& defaultValue = "");
int GetIntOption(const Napi::Object& obj, const char* key, int defaultValue = 0);
bool GetBoolOption(const Napi::Object& obj, const char* key, bool defaultValue = false);

} // namespace libssh_node

#endif // LIBSSH_NODE_UTILS_H
