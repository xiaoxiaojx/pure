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

void PerProcessOptions::CheckOptions(std::vector<std::string>* errors) {
  // TODO
}

void PerIsolateOptions::CheckOptions(std::vector<std::string>* errors) {
  per_env->CheckOptions(errors);
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
      // 和 push_back() 相同，都是在 vector 容器的尾部添加一个元素
      env_argv.emplace_back(std::string(1, c));
      will_start_new_arg = false;
    } else {
      // 返回尾元素的引用
      env_argv.back() += c;
    }
  }

  if (is_in_string) {
    errors->push_back("invalid value for PURE_OPTIONS "
                      "(unterminated string)\n");
  }
  return env_argv;
}

namespace options_parser {
class DebugOptionsParser : public OptionsParser<DebugOptions> {
 public:
  DebugOptionsParser();
};

class EnvironmentOptionsParser : public OptionsParser<EnvironmentOptions> {
 public:
  EnvironmentOptionsParser();
  explicit EnvironmentOptionsParser(const DebugOptionsParser& dop)
      : EnvironmentOptionsParser() {
  }
};

class PerIsolateOptionsParser : public OptionsParser<PerIsolateOptions> {
 public:
  PerIsolateOptionsParser() = delete;
  explicit PerIsolateOptionsParser(const EnvironmentOptionsParser& eop);
};

class PerProcessOptionsParser : public OptionsParser<PerProcessOptions> {
 public:
  PerProcessOptionsParser() = delete;
  explicit PerProcessOptionsParser(const PerIsolateOptionsParser& iop);
};

const EnvironmentOptionsParser _eop_instance{};
const PerIsolateOptionsParser _piop_instance{_eop_instance};
const PerProcessOptionsParser _ppop_instance{_piop_instance};

template <>
void Parse(StringVector* const args,
           StringVector* const exec_args,
           StringVector* const v8_args,
           PerIsolateOptions* const options,
           OptionEnvvarSettings required_env_settings,
           StringVector* const errors) {
  _piop_instance.Parse(
      args, exec_args, v8_args, options, required_env_settings, errors);
}

template <>
void Parse(StringVector* const args,
           StringVector* const exec_args,
           StringVector* const v8_args,
           PerProcessOptions* const options,
           OptionEnvvarSettings required_env_settings,
           StringVector* const errors) {
  _ppop_instance.Parse(
      args, exec_args, v8_args, options, required_env_settings, errors);
}

DebugOptionsParser::DebugOptionsParser() {
}

EnvironmentOptionsParser::EnvironmentOptionsParser() {
}

PerIsolateOptionsParser::PerIsolateOptionsParser(
    const EnvironmentOptionsParser& eop) {
}

PerProcessOptionsParser::PerProcessOptionsParser(
    const PerIsolateOptionsParser& iop) {
}

inline std::string RemoveBrackets(const std::string& host) {
  if (!host.empty() && host.front() == '[' && host.back() == ']')
    return host.substr(1, host.size() - 2);
  else
    return host;
}

inline int ParseAndValidatePort(const std::string& port,
                                std::vector<std::string>* errors) {
  char* endptr;
  errno = 0;
  const unsigned long result =                 // NOLINT(runtime/int)
    strtoul(port.c_str(), &endptr, 10);
  if (errno != 0 || *endptr != '\0'||
      (result != 0 && result < 1024) || result > 65535) {
    errors->push_back(" must be 0 or in range 1024 to 65535.");
  }
  return static_cast<int>(result);
}


HostPort SplitHostPort(const std::string& arg,
                      std::vector<std::string>* errors) {
  // remove_brackets only works if no port is specified
  // so if it has an effect only an IPv6 address was specified.
  std::string host = RemoveBrackets(arg);
  if (host.length() < arg.length())
    return HostPort{host, DebugOptions::kDefaultInspectorPort};

  size_t colon = arg.rfind(':');
  if (colon == std::string::npos) {
    // Either a port number or a host name.  Assume that
    // if it's not all decimal digits, it's a host name.
    for (char c : arg) {
      if (c < '0' || c > '9') {
        return HostPort{arg, DebugOptions::kDefaultInspectorPort};
      }
    }
    return HostPort { "", ParseAndValidatePort(arg, errors) };
  }
  // Host and port found:
  return HostPort { RemoveBrackets(arg.substr(0, colon)),
                    ParseAndValidatePort(arg.substr(colon + 1), errors) };
}

}  // namespace options_parser
}  // namespace pure