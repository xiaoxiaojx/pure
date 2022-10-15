#ifndef SRC_PURE_OPTIONS_H_
#define SRC_PURE_OPTIONS_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "env.h"
#include "pure_mutex.h"

namespace pure
{
    class PerProcessOptions;
    class PerIsolateOptions;

    namespace per_process
    {

        extern Mutex cli_options_mutex;
        extern PURE_EXTERN_PRIVATE std::shared_ptr<PerProcessOptions> cli_options;

    }

    class Options
    {
    public:
        virtual void CheckOptions(std::vector<std::string> *errors) {}
        virtual ~Options() = default;
    };

    class EnvironmentOptions : public Options
    {
    public:
        bool abort_on_uncaught_exception = false;
        std::vector<std::string> conditions;
        std::string dns_result_order;
        bool enable_source_maps = false;
        bool experimental_fetch = true;
        bool experimental_global_web_crypto = false;
        bool experimental_https_modules = false;
        std::string experimental_specifier_resolution;
        bool experimental_wasm_modules = false;
        bool experimental_import_meta_resolve = false;
        std::string module_type;
        std::string experimental_policy;
        std::string experimental_policy_integrity;
        bool has_policy_integrity_string = false;
        bool experimental_repl_await = true;
        bool experimental_vm_modules = false;
        bool expose_internals = false;
        bool force_node_api_uncaught_exceptions_policy = false;
        bool frozen_intrinsics = false;
        int64_t heap_snapshot_near_heap_limit = 0;
        std::string heap_snapshot_signal;
        uint64_t max_http_header_size = 16 * 1024;
        bool deprecation = true;
        bool force_async_hooks_checks = true;
        bool allow_native_addons = true;
        bool global_search_paths = true;
        bool warnings = true;
        bool force_context_aware = false;
        bool pending_deprecation = false;
        bool preserve_symlinks = false;
        bool preserve_symlinks_main = false;
        bool prof_process = false;
#if HAVE_INSPECTOR
        std::string cpu_prof_dir;
        static const uint64_t kDefaultCpuProfInterval = 1000;
        uint64_t cpu_prof_interval = kDefaultCpuProfInterval;
        std::string cpu_prof_name;
        bool cpu_prof = false;
        std::string heap_prof_dir;
        std::string heap_prof_name;
        static const uint64_t kDefaultHeapProfInterval = 512 * 1024;
        uint64_t heap_prof_interval = kDefaultHeapProfInterval;
        bool heap_prof = false;
#endif // HAVE_INSPECTOR
        std::string redirect_warnings;
        std::string diagnostic_dir;
        bool test_runner = false;
        bool test_only = false;
        bool test_udp_no_try_send = false;
        bool throw_deprecation = false;
        bool trace_atomics_wait = false;
        bool trace_deprecation = false;
        bool trace_exit = false;
        bool trace_sync_io = false;
        bool trace_tls = false;
        bool trace_uncaught = false;
        bool trace_warnings = false;
        bool extra_info_on_fatal_exception = true;
        std::string unhandled_rejections;
        std::vector<std::string> userland_loaders;
        bool verify_base_objects =
#ifdef DEBUG
            true;
#else
            false;
#endif // DEBUG

        bool syntax_check_only = false;
        bool has_eval_string = false;
        bool experimental_wasi = false;
        std::string eval_string;
        bool print_eval = false;
        bool force_repl = false;

        bool insecure_http_parser = false;

        bool tls_min_v1_0 = false;
        bool tls_min_v1_1 = false;
        bool tls_min_v1_2 = false;
        bool tls_min_v1_3 = false;
        bool tls_max_v1_2 = false;
        bool tls_max_v1_3 = false;
        std::string tls_keylog;

        std::vector<std::string> preload_modules;

        std::vector<std::string> user_argv;

        void CheckOptions(std::vector<std::string> *errors) override;
    };

    class PerIsolateOptions : public Options
    {
    public:
        std::shared_ptr<EnvironmentOptions> per_env{new EnvironmentOptions()};
        bool track_heap_objects = false;
        bool report_uncaught_exception = false;
        bool report_on_signal = false;
        bool experimental_shadow_realm = false;
        std::string report_signal = "SIGUSR2";
        inline EnvironmentOptions *get_per_env_options();
        void CheckOptions(std::vector<std::string> *errors) override;
    };

    class PerProcessOptions : public Options
    {
    public:
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

        std::vector<std::string> security_reverts;
        bool print_bash_completion = false;
        bool print_help = false;
        bool print_v8_help = false;
        bool print_version = false;

#ifdef NODE_HAVE_I18N_SUPPORT
        std::string icu_data_dir;
#endif

        // Per-process because they affect singleton OpenSSL shared library state,
        // or are used once during process initialization.
#if HAVE_OPENSSL
        std::string openssl_config;
        std::string tls_cipher_list = DEFAULT_CIPHER_LIST_CORE;
        int64_t secure_heap = 0;
        int64_t secure_heap_min = 2;
#ifdef NODE_OPENSSL_CERT_STORE
        bool ssl_openssl_cert_store = true;
#else
        bool ssl_openssl_cert_store = false;
#endif
        bool use_openssl_ca = false;
        bool use_bundled_ca = false;
        bool enable_fips_crypto = false;
        bool force_fips_crypto = false;
#endif
#if OPENSSL_VERSION_MAJOR >= 3
        bool openssl_legacy_provider = false;
        bool openssl_shared_config = false;
#endif

        // Per-process because reports can be triggered outside a known V8 context.
        bool report_on_fatalerror = false;
        bool report_compact = false;
        std::string report_directory;
        std::string report_filename;

        // TODO(addaleax): Some of these could probably be per-Environment.
        std::string use_largepages = "off";
        bool trace_sigint = false;
        std::vector<std::string> cmdline;

        inline PerIsolateOptions *get_per_isolate_options();
        void CheckOptions(std::vector<std::string> *errors) override;
    };
}

#endif // SRC_PURE_OPTIONS_H_