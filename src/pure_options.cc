#include "pure_options.h"
#include <cstdlib>
#include "pure_mutex.h"
#include "pure_options-inl.h"

namespace pure {

namespace per_process {
Mutex cli_options_mutex;
std::shared_ptr<PerProcessOptions> cli_options{new PerProcessOptions()};
}  // namespace per_process

void EnvironmentOptions::CheckOptions(std::vector<std::string>* errors) {
  // TODO
}

void HandleEnvOptions(std::shared_ptr<EnvironmentOptions> env_options) {
  HandleEnvOptions(env_options, [](const char* name) {
    std::string text;
    return credentials::SafeGetenv(name, &text) ? text : "";
  });
}

void HandleEnvOptions(std::shared_ptr<EnvironmentOptions> env_options,
                      std::function<std::string(const char*)> opt_getter) {
  // TODO
  //   env_options->pending_deprecation =
  //       opt_getter("NODE_PENDING_DEPRECATION") == "1";

  //   env_options->preserve_symlinks = opt_getter("NODE_PRESERVE_SYMLINKS") ==
  //   "1";

  //   env_options->preserve_symlinks_main =
  //       opt_getter("NODE_PRESERVE_SYMLINKS_MAIN") == "1";

  //   if (env_options->redirect_warnings.empty())
  //     env_options->redirect_warnings = opt_getter("NODE_REDIRECT_WARNINGS");
}

std::vector<std::string> ParseNodeOptionsEnvVar(
    const std::string& pure_options, std::vector<std::string>* errors) {
  std::vector<std::string> env_argv;

  bool is_in_string = false;
  bool will_start_new_arg = true;
  for (std::string::size_type index = 0; index < pure_options.size(); ++index) {
    char c = pure_options.at(index);

    // Backslashes escape the following character
    if (c == '\\' && is_in_string) {
      if (index + 1 == pure_options.size()) {
        errors->push_back("invalid value for PURE_OPTIONS "
                          "(invalid escape)\n");
        return env_argv;
      } else {
        c = pure_options.at(++index);
      }
    } else if (c == ' ' && !is_in_string) {
      will_start_new_arg = true;
      continue;
    } else if (c == '"') {
      is_in_string = !is_in_string;
      continue;
    }

    if (will_start_new_arg) {
      env_argv.emplace_back(std::string(1, c));
      will_start_new_arg = false;
    } else {
      env_argv.back() += c;
    }
  }

  if (is_in_string) {
    errors->push_back("invalid value for PURE_OPTIONS "
                      "(unterminated string)\n");
  }
  return env_argv;
}
}  // namespace pure