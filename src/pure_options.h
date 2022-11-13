#ifndef SRC_PURE_OPTIONS_H_
#define SRC_PURE_OPTIONS_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "pure.h"
#include "pure_mutex.h"

namespace pure {

#define SECURITY_REVERSIONS(XX)                                                \
  //  XX(CVE_2016_PEND, "CVE-2016-PEND", "Vulnerability Title")

enum reversion {
#define V(code, ...) SECURITY_REVERT_##code,
  SECURITY_REVERSIONS(V)
#undef V
};

namespace per_process {
extern unsigned int reverted_cve;
}

#ifdef _MSC_VER
#pragma warning(push)
// MSVC C4065: switch statement contains 'default' but no 'case' labels
#pragma warning(disable : 4065)
#endif

inline const char* RevertMessage(const reversion cve) {
#define V(code, label, msg)                                                    \
  case SECURITY_REVERT_##code:                                                 \
    return label ": " msg;
  switch (cve) {
    SECURITY_REVERSIONS(V)
    default:
      return "Unknown";
  }
#undef V
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

inline void Revert(const reversion cve) {
  per_process::reverted_cve |= 1 << cve;
  printf("SECURITY WARNING: Reverting %s\n", RevertMessage(cve));
}

inline void Revert(const char* cve, std::string* error) {
#define V(code, label, _)                                                      \
  if (strcmp(cve, label) == 0) return Revert(SECURITY_REVERT_##code);
  SECURITY_REVERSIONS(V)
#undef V
  *error = "Error: Attempt to revert an unknown CVE [";
  *error += cve;
  *error += ']';
}

inline bool IsReverted(const reversion cve) {
  return per_process::reverted_cve & (1 << cve);
}

inline bool IsReverted(const char* cve) {
#define V(code, label, _)                                                      \
  if (strcmp(cve, label) == 0) return IsReverted(SECURITY_REVERT_##code);
  SECURITY_REVERSIONS(V)
  return false;
#undef V
}

class HostPort {
 public:
  HostPort(const std::string& host_name, int port)
      : host_name_(host_name), port_(port) {}
  HostPort(const HostPort&) = default;
  HostPort& operator=(const HostPort&) = default;
  HostPort(HostPort&&) = default;
  HostPort& operator=(HostPort&&) = default;

  void set_host(const std::string& host) { host_name_ = host; }

  void set_port(int port) { port_ = port; }

  const std::string& host() const { return host_name_; }

  int port() const {
    // TODO(joyeecheung): make port a uint16_t
    CHECK_GE(port_, 0);
    return port_;
  }

  void Update(const HostPort& other) {
    if (!other.host_name_.empty()) host_name_ = other.host_name_;
    if (other.port_ >= 0) port_ = other.port_;
  }

 private:
  std::string host_name_;
  int port_;
};

class Options {
 public:
  virtual void CheckOptions(std::vector<std::string>* errors) {}
  virtual ~Options() = default;
};

struct InspectPublishUid {
  bool console;
  bool http;
};

// These options are currently essentially per-Environment, but it can be nice
// to keep them separate since they are a group of options applying to a very
// specific part of Node. It might also make more sense for them to be
// per-Isolate, rather than per-Environment.
class DebugOptions : public Options {
 public:
  DebugOptions() = default;
  DebugOptions(const DebugOptions&) = default;
  DebugOptions& operator=(const DebugOptions&) = default;
  DebugOptions(DebugOptions&&) = default;
  DebugOptions& operator=(DebugOptions&&) = default;

  // --inspect
  bool inspector_enabled = false;
  // --debug
  bool deprecated_debug = false;
  // --inspect-brk
  bool break_first_line = false;
  // --inspect-brk-node
  bool break_node_first_line = false;
  // --inspect-publish-uid
  std::string inspect_publish_uid_string = "stderr,http";

  InspectPublishUid inspect_publish_uid;

  enum { kDefaultInspectorPort = 9229 };

  HostPort host_port{"127.0.0.1", kDefaultInspectorPort};

  // Used to patch the options as if --inspect-brk is passed.
  void EnableBreakFirstLine() {
    inspector_enabled = true;
    break_first_line = true;
  }

  bool wait_for_connect() const {
    return break_first_line || break_node_first_line;
  }

  void CheckOptions(std::vector<std::string>* errors) override;
};

class EnvironmentOptions : public Options {
 public:
  // TODO
  bool abort_on_uncaught_exception = false;

  std::vector<std::string> user_argv;
  bool pending_deprecation = false;

  void CheckOptions(std::vector<std::string>* errors) override;
};

class PerIsolateOptions : public Options {
 public:
  std::shared_ptr<EnvironmentOptions> per_env{new EnvironmentOptions()};
  // TODO
  inline EnvironmentOptions* get_per_env_options();
  void CheckOptions(std::vector<std::string>* errors) override;
};

class PerProcessOptions : public Options {
 public:
  // TODO
  // Options shouldn't be here unless they affect the entire process scope, and
  // that should avoided when possible.
  //
  // When an option is used during process initialization, it does not need
  // protection, but any use after that will likely require synchronization
  // using the node::per_process::cli_options_mutex, typically:
  //
  //     Mutex::ScopedLock lock(node::per_process::cli_options_mutex);
  std::shared_ptr<PerIsolateOptions> per_isolate{new PerIsolateOptions()};

  std::string title;
  std::string trace_event_categories;
  std::string trace_event_file_pattern = "node_trace.${rotation}.log";
  int64_t v8_thread_pool_size = 4;
  bool zero_fill_all_buffers = false;
  bool debug_arraybuffer_allocations = false;
  std::string disable_proto;
  bool build_snapshot = false;
  // We enable the shared read-only heap which currently requires that the
  // snapshot used in different isolates in the same process to be the same.
  // Therefore --node-snapshot is a per-process option.
  bool node_snapshot = true;
  bool report_on_fatalerror = false;

  std::vector<std::string> security_reverts;
  bool print_bash_completion = false;
  bool print_help = false;
  bool print_v8_help = false;
  bool print_version = false;
  std::vector<std::string> cmdline;

  inline PerIsolateOptions* get_per_isolate_options();
  void CheckOptions(std::vector<std::string>* errors) override;
};

void HandleEnvOptions(std::shared_ptr<EnvironmentOptions> env_options);
void HandleEnvOptions(std::shared_ptr<EnvironmentOptions> env_options,
                      std::function<std::string(const char*)> opt_getter);

std::vector<std::string> ParseNodeOptionsEnvVar(
    const std::string& node_options, std::vector<std::string>* errors);

namespace options_parser {

HostPort SplitHostPort(const std::string& arg,
                       std::vector<std::string>* errors);
void GetOptions(const v8::FunctionCallbackInfo<v8::Value>& args);
std::string GetBashCompletion();

enum OptionType {
  kNoOp,
  kV8Option,
  kBoolean,
  kInteger,
  kUInteger,
  kString,
  kHostPort,
  kStringList,
};

template <typename Options>
class OptionsParser {
 public:
  virtual ~OptionsParser() = default;

  typedef Options TargetType;

  struct NoOp {};
  struct V8Option {};

  // These methods add a single option to the parser. Optionally, it can be
  // specified whether the option should be allowed from environment variable
  // sources (i.e. NODE_OPTIONS).
  void AddOption(const char* name,
                 const char* help_text,
                 bool Options::*field,
                 OptionEnvvarSettings env_setting = kDisallowedInEnvironment,
                 bool default_is_true = false);
  void AddOption(const char* name,
                 const char* help_text,
                 uint64_t Options::*field,
                 OptionEnvvarSettings env_setting = kDisallowedInEnvironment);
  void AddOption(const char* name,
                 const char* help_text,
                 int64_t Options::*field,
                 OptionEnvvarSettings env_setting = kDisallowedInEnvironment);
  void AddOption(const char* name,
                 const char* help_text,
                 std::string Options::*field,
                 OptionEnvvarSettings env_setting = kDisallowedInEnvironment);
  void AddOption(const char* name,
                 const char* help_text,
                 std::vector<std::string> Options::*field,
                 OptionEnvvarSettings env_setting = kDisallowedInEnvironment);
  void AddOption(const char* name,
                 const char* help_text,
                 HostPort Options::*field,
                 OptionEnvvarSettings env_setting = kDisallowedInEnvironment);
  void AddOption(const char* name,
                 const char* help_text,
                 NoOp no_op_tag,
                 OptionEnvvarSettings env_setting = kDisallowedInEnvironment);
  void AddOption(const char* name,
                 const char* help_text,
                 V8Option v8_option_tag,
                 OptionEnvvarSettings env_setting = kDisallowedInEnvironment);

  // Adds aliases. An alias can be of the form "--option-a" -> "--option-b",
  // or have a more complex group expansion, like
  //   "--option-a" -> { "--option-b", "--harmony-foobar", "--eval", "42" }
  // If `from` has the form "--option-a=", the alias will only be expanded if
  // the option is presented in that form (i.e. with a '=').
  // If `from` has the form "--option-a <arg>", the alias will only be expanded
  // if the option has a non-option argument (not starting with -) following it.
  void AddAlias(const char* from, const char* to);
  void AddAlias(const char* from, const std::vector<std::string>& to);
  void AddAlias(const char* from, const std::initializer_list<std::string>& to);

  // Add implications from some arbitrary option to a boolean one, either
  // in a way that makes `from` set `to` to true or to false.
  void Implies(const char* from, const char* to);
  void ImpliesNot(const char* from, const char* to);

  // Insert options from another options parser into this one, along with
  // a method that yields the target options type from this parser's options
  // type.
  template <typename ChildOptions>
  void Insert(const OptionsParser<ChildOptions>& child_options_parser,
              ChildOptions* (Options::*get_child)());

  // Parse a sequence of options into an options struct, a list of
  // arguments that were parsed as options, a list of unknown/JS engine options,
  // and leave the remainder in the input `args` vector.
  //
  // For example, an `args` input of
  //
  //   node --foo --harmony-bar --fizzle=42 -- /path/to/cow moo
  //
  // expands as
  //
  // - `args` -> { "node", "/path/to/cow", "moo" }
  // - `exec_args` -> { "--foo", "--harmony-bar", "--fizzle=42" }
  // - `v8_args` -> `{ "node", "--harmony-bar" }
  // - `options->foo == true`, `options->fizzle == 42`.
  //
  // If `*error` is set, the result of the parsing should be discarded and the
  // contents of any of the argument vectors should be considered undefined.
  void Parse(std::vector<std::string>* const args,
             std::vector<std::string>* const exec_args,
             std::vector<std::string>* const v8_args,
             Options* const options,
             OptionEnvvarSettings required_env_settings,
             std::vector<std::string>* const errors) const;

 private:
  // We support the wide variety of different option types by remembering
  // how to access them, given a certain `Options` struct.

  // Represents a field within `Options`.
  class BaseOptionField {
   public:
    virtual ~BaseOptionField() = default;
    virtual void* LookupImpl(Options* options) const = 0;

    template <typename T>
    inline T* Lookup(Options* options) const {
      return static_cast<T*>(LookupImpl(options));
    }
  };

  // Represents a field of type T within `Options` that can be looked up
  // as a C++ member field.
  template <typename T>
  class SimpleOptionField : public BaseOptionField {
   public:
    explicit SimpleOptionField(T Options::*field) : field_(field) {}
    void* LookupImpl(Options* options) const override {
      return static_cast<void*>(&(options->*field_));
    }

   private:
    T Options::*field_;
  };

  template <typename T>
  inline T* Lookup(std::shared_ptr<BaseOptionField> field,
                   Options* options) const {
    return field->template Lookup<T>(options);
  }

  // An option consists of:
  // - A type.
  // - A way to store/access the property value.
  // - The information of whether it may occur in an env var or not.
  struct OptionInfo {
    OptionType type;
    std::shared_ptr<BaseOptionField> field;
    OptionEnvvarSettings env_setting;
    std::string help_text;
    bool default_is_true = false;
  };

  // An implied option is composed of the information on where to store a
  // specific boolean value (if another specific option is encountered).
  struct Implication {
    OptionType type;
    std::string name;
    std::shared_ptr<BaseOptionField> target_field;
    bool target_value;
  };

  // These are helpers that make `Insert()` support properties of other
  // options structs, if we know how to access them.
  template <typename OriginalField, typename ChildOptions>
  static auto Convert(std::shared_ptr<OriginalField> original,
                      ChildOptions* (Options::*get_child)());
  template <typename ChildOptions>
  static auto Convert(typename OptionsParser<ChildOptions>::OptionInfo original,
                      ChildOptions* (Options::*get_child)());
  template <typename ChildOptions>
  static auto Convert(
      typename OptionsParser<ChildOptions>::Implication original,
      ChildOptions* (Options::*get_child)());

  std::unordered_map<std::string, OptionInfo> options_;
  std::unordered_map<std::string, std::vector<std::string>> aliases_;
  std::unordered_multimap<std::string, Implication> implications_;

  template <typename OtherOptions>
  friend class OptionsParser;

  friend void GetCLIOptions(const v8::FunctionCallbackInfo<v8::Value>& args);
  friend std::string GetBashCompletion();
};

using StringVector = std::vector<std::string>;
template <class OptionsType, class = Options>
void Parse(StringVector* const args,
           StringVector* const exec_args,
           StringVector* const v8_args,
           OptionsType* const options,
           OptionEnvvarSettings required_env_settings,
           StringVector* const errors);

}  // namespace options_parser

namespace per_process {

extern Mutex cli_options_mutex;
extern PURE_EXTERN_PRIVATE std::shared_ptr<PerProcessOptions> cli_options;

}  // namespace per_process
}  // namespace pure

#endif  // SRC_PURE_OPTIONS_H_