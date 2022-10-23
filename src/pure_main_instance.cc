#include <assert.h>

#include "pure_main_instance.h"
#include "pure_mutex.h"
#include "uv.h"
#include "v8.h"

#include "env.h"
#include "pure.h"

#include "env-inl.h"

#include "pure_errors.h"
#include "pure_options.h"

namespace pure {

using v8::Context;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Locker;
using v8::String;

using v8::Isolate;
using v8::Local;
using v8::Object;

// Local<Context> NewContext(Isolate *);

static std::atomic<uint64_t> next_thread_id{0};

void PromiseRejectCallback(v8::PromiseRejectMessage message) {
  // TODO
}

void SetIsolateMiscHandlers(v8::Isolate* isolate, const IsolateSettings& s) {
  isolate->SetMicrotasksPolicy(s.policy);

  // auto *allow_wasm_codegen_cb = s.allow_wasm_code_generation_callback ?
  // s.allow_wasm_code_generation_callback : AllowWasmCodeGenerationCallback;
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

  if ((s.flags & SHOULD_NOT_SET_PROMISE_REJECTION_CALLBACK) == 0) {
    auto* promise_reject_cb = s.promise_reject_callback
                                  ? s.promise_reject_callback
                                  : PromiseRejectCallback;
    isolate->SetPromiseRejectCallback(promise_reject_cb);
  }

  // if (s.flags & DETAILED_SOURCE_POSITIONS_FOR_PROFILING)
  //   v8::CpuProfiler::UseDetailedSourcePositionsForProfiling(isolate);
}

void SetIsolateErrorHandlers(v8::Isolate* isolate, const IsolateSettings& s) {
  if (s.flags & MESSAGE_LISTENER_WITH_ERROR_LEVEL)
    isolate->AddMessageListenerWithErrorLevel(
        errors::PerIsolateMessageListener,
        Isolate::MessageErrorLevel::kMessageError |
            Isolate::MessageErrorLevel::kMessageWarning);

  auto* abort_callback = s.should_abort_on_uncaught_exception_callback
                             ? s.should_abort_on_uncaught_exception_callback
                             : ShouldAbortOnUncaughtException;
  isolate->SetAbortOnUncaughtExceptionCallback(abort_callback);

  auto* fatal_error_cb = s.fatal_error_callback ? s.fatal_error_callback
                                                : pure::errors::OnFatalError;
  isolate->SetFatalErrorHandler(fatal_error_cb);

  // if ((s.flags & SHOULD_NOT_SET_PREPARE_STACK_TRACE_CALLBACK) == 0)
  // {
  //   auto *prepare_stack_trace_cb = s.prepare_stack_trace_callback ?
  //   s.prepare_stack_trace_callback : PrepareStackTraceCallback;
  //   isolate->SetPrepareStackTraceCallback(prepare_stack_trace_cb);
  // }
}

PureMainInstance::PureMainInstance(uv_loop_t* event_loop,
                                   const std::vector<std::string>& args,
                                   const std::vector<std::string>& exec_args)
    : args_(args),
      exec_args_(exec_args),
      isolate_(nullptr),
      isolate_data_(),
      isolate_params_(std::make_unique<Isolate::CreateParams>()) {
  isolate_params_->array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();

  platform_ = per_process::v8_platform.get();
  isolate_ = Isolate::Allocate();
  assert(isolate_);
  // Register the isolate on the platform before the isolate gets initialized,
  // so that the isolate can access the platform during initialization.
  // platform->RegisterIsolate(isolate_, event_loop);
  Isolate::Initialize(isolate_, *isolate_params_);

  isolate_data_ =
      std::make_unique<IsolateData>(isolate_, event_loop, platform_);
  IsolateSettings s;
  SetIsolateMiscHandlers(isolate_, s);
  SetIsolateErrorHandlers(isolate_, s);
  // isolate_data_->max_young_gen_size =
  //     isolate_params_->constraints.max_young_generation_size_in_bytes();
};

PureMainInstance::~PureMainInstance() {
  if (isolate_params_ == nullptr) {
    return;
  }
  // This should only be done on a main instance that owns its isolate.
  isolate_->Dispose();
}

DeleteFnPtr<Environment, FreeEnvironment>
PureMainInstance::CreateMainEnvironment(int* exit_code) {
  *exit_code = 0;  // Reset the exit code to 0

  HandleScope handle_scope(isolate_);

  // TODO(addaleax): This should load a real per-Isolate option, currently
  // this is still effectively per-process.
  // if (isolate_data_->options()->track_heap_objects)
  // {
  //   isolate_->GetHeapProfiler()->StartTrackingHeapObjects(true);
  // }

  Local<Context> context;
  DeleteFnPtr<Environment, FreeEnvironment> env;

  context = NewContext(isolate_);
  CHECK(!context.IsEmpty());
  Context::Scope context_scope(context);
  env.reset(new Environment(isolate_data_.get(),
                            context,
                            args_,
                            exec_args_,
                            EnvironmentFlags::kDefaultFlags));

  if (env->RunBootstrapping().IsEmpty()) {
    return nullptr;
  }
  return env;
}

int PureMainInstance::Run() {
  Locker locker(isolate_);
  Isolate::Scope isolate_scope(isolate_);
  HandleScope handle_scope(isolate_);

  int exit_code = 0;
  DeleteFnPtr<Environment, FreeEnvironment> env =
      CreateMainEnvironment(&exit_code);
  CHECK_NOT_NULL(env);

  Context::Scope context_scope(env->context());
  Run(&exit_code, env.get());
  return exit_code;
}

void PureMainInstance::Run(int* exit_code, Environment* env) {
  if (*exit_code == 0) {
    LoadEnvironment(env, StartExecutionCallback{});

    // *exit_code = SpinEventLoop(env).FromMaybe(1);
  }

  ResetStdio();

#if defined(LEAK_SANITIZER)
  __lsan_do_leak_check();
#endif
}
}  // namespace pure
