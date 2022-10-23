#ifndef SRC_PURE_OPTIONS_H_
#define SRC_PURE_OPTIONS_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "pure.h"
#include "pure_mutex.h"

namespace pure {
class PerProcessOptions;
class PerIsolateOptions;

namespace per_process {

extern Mutex cli_options_mutex;
extern PURE_EXTERN_PRIVATE std::shared_ptr<PerProcessOptions> cli_options;

}  // namespace per_process

class Options {
 public:
  virtual void CheckOptions(std::vector<std::string>* errors) {}
  virtual ~Options() = default;
};

class EnvironmentOptions : public Options {
 public:
  // TODO
  bool abort_on_uncaught_exception = false;

  std::vector<std::string> user_argv;

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
  inline PerIsolateOptions* get_per_isolate_options();
  void CheckOptions(std::vector<std::string>* errors) override;
};
}  // namespace pure

#endif  // SRC_PURE_OPTIONS_H_