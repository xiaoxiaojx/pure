#include <assert.h>

#include "v8.h"
#include "uv.h"
#include "pure_main_instance.h"
#include "pure_mutex.h"

#include "env.h"
#include "pure_options.h"
#include "pure_errors.h"

using v8::Isolate;

namespace pure
{

  static std::atomic<uint64_t> next_thread_id{0};

  void PromiseRejectCallback(v8::PromiseRejectMessage message)
  {
    // TODO
  }

  void SetIsolateMiscHandlers(v8::Isolate *isolate, const IsolateSettings &s)
  {
    isolate->SetMicrotasksPolicy(s.policy);

    // auto *allow_wasm_codegen_cb = s.allow_wasm_code_generation_callback ? s.allow_wasm_code_generation_callback : AllowWasmCodeGenerationCallback;
    // isolate->SetAllowWasmCodeGenerationCallback(allow_wasm_codegen_cb);

    Mutex::ScopedLock lock(pure::per_process::cli_options_mutex);
    // if (per_process::cli_options->get_per_isolate_options()
    //         ->get_per_env_options()
    //         ->experimental_fetch)
    // {
    //   isolate->SetWasmStreamingCallback(wasm_web_api::StartStreamingCompilation);
    // }

    // if (per_process::cli_options->get_per_isolate_options()
    //         ->experimental_shadow_realm)
    // {
    //   isolate->SetHostCreateShadowRealmContextCallback(
    //       shadow_realm::HostCreateShadowRealmContextCallback);
    // }

    if ((s.flags & SHOULD_NOT_SET_PROMISE_REJECTION_CALLBACK) == 0)
    {
      auto *promise_reject_cb = s.promise_reject_callback ? s.promise_reject_callback : PromiseRejectCallback;
      isolate->SetPromiseRejectCallback(promise_reject_cb);
    }

    // if (s.flags & DETAILED_SOURCE_POSITIONS_FOR_PROFILING)
    //   v8::CpuProfiler::UseDetailedSourcePositionsForProfiling(isolate);
  }

  void SetIsolateErrorHandlers(v8::Isolate *isolate, const IsolateSettings &s)
  {
    if (s.flags & MESSAGE_LISTENER_WITH_ERROR_LEVEL)
      isolate->AddMessageListenerWithErrorLevel(
          errors::PerIsolateMessageListener,
          Isolate::MessageErrorLevel::kMessageError |
              Isolate::MessageErrorLevel::kMessageWarning);

    // auto *abort_callback = s.should_abort_on_uncaught_exception_callback ? s.should_abort_on_uncaught_exception_callback : ShouldAbortOnUncaughtException;
    // isolate->SetAbortOnUncaughtExceptionCallback(abort_callback);

    // auto *fatal_error_cb = s.fatal_error_callback ? s.fatal_error_callback : OnFatalError;
    // isolate->SetFatalErrorHandler(fatal_error_cb);

    // if ((s.flags & SHOULD_NOT_SET_PREPARE_STACK_TRACE_CALLBACK) == 0)
    // {
    //   auto *prepare_stack_trace_cb = s.prepare_stack_trace_callback ? s.prepare_stack_trace_callback : PrepareStackTraceCallback;
    //   isolate->SetPrepareStackTraceCallback(prepare_stack_trace_cb);
    // }
  }

  PureMainInstance::PureMainInstance(
      uv_loop_t *event_loop,
      MultiIsolatePlatform *platform,
      const std::vector<std::string> &args,
      const std::vector<std::string> &exec_args)
      : args_(args),
        exec_args_(exec_args),
        isolate_(nullptr),
        platform_(platform),
        isolate_data_(),
        isolate_params_(std::make_unique<Isolate::CreateParams>())
  {
    isolate_ = Isolate::Allocate();
    assert(isolate_);
    // Register the isolate on the platform before the isolate gets initialized,
    // so that the isolate can access the platform during initialization.
    platform->RegisterIsolate(isolate_, event_loop);
    Isolate::Initialize(isolate_, *isolate_params_);

    // If the indexes are not nullptr, we are not deserializing
    isolate_data_ = std::make_unique<IsolateData>(
        isolate_,
        event_loop,
        platform);
    IsolateSettings s;
    SetIsolateMiscHandlers(isolate_, s);
    // if (snapshot_data == nullptr)
    // {
    // If in deserialize mode, delay until after the deserialization is
    // complete.
    SetIsolateErrorHandlers(isolate_, s);
    // }
    // isolate_data_->max_young_gen_size =
    //     isolate_params_->constraints.max_young_generation_size_in_bytes();
  };
}
