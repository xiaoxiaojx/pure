#include "env.h"
#include "env-inl.h"

#include "util-inl.h"
#include "util.h"
#include "uv.h"
#include "v8.h"

// #include "util.h"
#include <iostream>
#include "pure.h"
#include "pure_options.h"

#include "pure_binding.h"
#include "pure_native_module.h"

namespace pure {

using v8::Context;
using v8::EscapableHandleScope;
using v8::Function;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Isolate;
using v8::Just;
using v8::Local;
using v8::Maybe;
using v8::MaybeLocal;
using v8::Nothing;
using v8::Object;
using v8::Script;
using v8::SealHandleScope;
using v8::String;
using v8::Value;

IsolateData::IsolateData(Isolate* isolate,
                         uv_loop_t* event_loop,
                         v8::Platform* platform)
    : isolate_(isolate), event_loop_(event_loop), platform_(platform) {
  options_.reset(
      new PerIsolateOptions(*(per_process::cli_options->per_isolate)));

  CreateProperties();
}

ImmediateInfo::ImmediateInfo(Isolate* isolate)
    : fields_(isolate, kFieldsCount, nullptr) {}

void IsolateData::CreateProperties() {
  // TODO
}

int const Environment::kNodeContextTag = 0x6e6f64;
void* const Environment::kNodeContextTagPtr =
    const_cast<void*>(static_cast<const void*>(&Environment::kNodeContextTag));

std::string GetExecPath(const std::vector<std::string>& argv) {
  char exec_path_buf[2 * PATH_MAX];
  size_t exec_path_len = sizeof(exec_path_buf);
  std::string exec_path;
  if (uv_exepath(exec_path_buf, &exec_path_len) == 0) {
    exec_path = std::string(exec_path_buf, exec_path_len);
  } else {
    exec_path = argv[0];
  }

  return exec_path;
}

void Environment::InitializeMainContext(Local<Context> context,
                                        const EnvSerializeInfo* env_info) {
  context_.Reset(context->GetIsolate(), context);
  AssignToContext(context, ContextInfo(""));
  // if (env_info != nullptr)
  // {
  //     DeserializeProperties(env_info);
  // }
  // else
  // {
  CreateProperties();
  // }
}

void Environment::CreateProperties() {
  HandleScope handle_scope(isolate_);
  Local<Context> ctx = context();

  {
    Context::Scope context_scope(ctx);
    Local<FunctionTemplate> templ = FunctionTemplate::New(isolate());
    // templ->InstanceTemplate()->SetInternalFieldCount(
    //     BaseObject::kInternalFieldCount);
    // templ->Inherit(BaseObject::GetConstructorTemplate(this));

    set_binding_data_ctor_template(templ);
  }
  // TODO
  // Store primordials setup by the per-context script in the environment.
  //         Local<Object> per_context_bindings =
  //             GetPerContextExports(ctx).ToLocalChecked();
  //         Local<Value> primordials =
  //             per_context_bindings->Get(ctx,
  //             primordials_string()).ToLocalChecked();
  //         CHECK(primordials->IsObject());
  //         set_primordials(primordials.As<Object>());

  //         Local<String> prototype_string =
  //             FIXED_ONE_BYTE_STRING(isolate(), "prototype");

  // #define V(EnvPropertyName, PrimordialsPropertyName)                              \
//     {                                                                            \
//         Local<Value> ctor =                                                      \
//             primordials.As<Object>()                                             \
//                 ->Get(ctx,                                                       \
//                       FIXED_ONE_BYTE_STRING(isolate(), PrimordialsPropertyName)) \
//                 .ToLocalChecked();                                               \
//         CHECK(ctor->IsObject());                                                 \
//         Local<Value> prototype =                                                 \
//             ctor.As<Object>()->Get(ctx, prototype_string).ToLocalChecked();      \
//         CHECK(prototype->IsObject());                                            \
//         set_##EnvPropertyName(prototype.As<Object>());                           \
//     }

  //         V(primordials_safe_map_prototype_object, "SafeMap");
  //         V(primordials_safe_set_prototype_object, "SafeSet");
  //         V(primordials_safe_weak_map_prototype_object, "SafeWeakMap");
  //         V(primordials_safe_weak_set_prototype_object, "SafeWeakSet");
  // #undef V

  //         Local<Object> process_object =
  //             node::CreateProcessObject(this).FromMaybe(Local<Object>());
  //         set_process_object(process_object);
}

Environment::Environment(IsolateData* isolate_data,
                         Isolate* isolate,
                         const std::vector<std::string>& args,
                         const std::vector<std::string>& exec_args,
                         EnvironmentFlags::Flags flags)
    : isolate_(isolate),
      isolate_data_(isolate_data),
      immediate_info_(isolate),
      //   tick_info_(isolate, MAYBE_FIELD_PTR(env_info, tick_info)),
      timer_base_(uv_now(isolate_data->event_loop())),
      exec_argv_(exec_args),
      argv_(args),
      exec_path_(GetExecPath(args)),
      //   should_abort_on_uncaught_toggle_(
      //       isolate_,
      //       1,
      //       MAYBE_FIELD_PTR(env_info, should_abort_on_uncaught_toggle)),
      //   stream_base_state_(isolate_,
      //                      StreamBase::kNumStreamBaseStateFields,
      //                      MAYBE_FIELD_PTR(env_info, stream_base_state)),
      environment_start_time_(PERFORMANCE_NOW()),
      flags_(flags) {
  // TODO
}

Environment::Environment(IsolateData* isolate_data,
                         Local<Context> context,
                         const std::vector<std::string>& args,
                         const std::vector<std::string>& exec_args,
                         EnvironmentFlags::Flags flags)
    : Environment(isolate_data, context->GetIsolate(), args, exec_args, flags) {
  InitializeMainContext(context, nullptr);
}

Environment::~Environment() {
  // TODO
}

void Environment::CleanupHandles() {
  {
    // Mutex::ScopedLock lock(native_immediates_threadsafe_mutex_);
    // task_queues_async_initialized_ = false;
  }

  // Assert that no Javascript code is invoked.
  Isolate::DisallowJavascriptExecutionScope disallow_js(
      isolate(), Isolate::DisallowJavascriptExecutionScope::THROW_ON_FAILURE);

  // RunAndClearNativeImmediates(true /* skip unrefed SetImmediate()s */);

  // for (ReqWrapBase *request : req_wrap_queue_)
  //     request->Cancel();

  // for (HandleWrap *handle : handle_wrap_queue_)
  //     handle->Close();

  // for (HandleCleanup &hc : handle_cleanup_queue_)
  //     hc.cb_(this, hc.handle_, hc.arg_);
  // handle_cleanup_queue_.clear();

  // while (handle_cleanup_waiting_ != 0 ||
  //        request_waiting_ != 0 ||
  //        !handle_wrap_queue_.IsEmpty())
  { uv_run(event_loop(), UV_RUN_ONCE); }
}

void Environment::AtExit(void (*cb)(void* arg), void* arg) {
  // TODO
  // at_exit_functions_.push_front(ExitCallback{cb, arg});
}

void Environment::RunCleanup() {
  started_cleanup_ = true;
  // TRACE_EVENT0(TRACING_CATEGORY_NODE1(environment), "RunCleanup");
  // bindings_.clear();
  CleanupHandles();

  // TODO
  //  while (!cleanup_hooks_.empty() ||
  //         native_immediates_.size() > 0 ||
  //         native_immediates_threadsafe_.size() > 0 ||
  //         native_immediates_interrupts_.size() > 0)
  //  {
  //      // Copy into a vector, since we can't sort an unordered_set in-place.
  //      std::vector<CleanupHookCallback> callbacks(
  //          cleanup_hooks_.begin(), cleanup_hooks_.end());
  //      // We can't erase the copied elements from `cleanup_hooks_` yet,
  //      because we
  //      // need to be able to check whether they were un-scheduled by another
  //      hook.

  //     std::sort(callbacks.begin(), callbacks.end(),
  //               [](const CleanupHookCallback &a, const CleanupHookCallback
  //               &b)
  //               {
  //                   // Sort in descending order so that the most recently
  //                   inserted callbacks
  //                   // are run first.
  //                   return a.insertion_order_counter_ >
  //                   b.insertion_order_counter_;
  //               });

  //     for (const CleanupHookCallback &cb : callbacks)
  //     {
  //         if (cleanup_hooks_.count(cb) == 0)
  //         {
  //             // This hook was removed from the `cleanup_hooks_` set during
  //             another
  //             // hook that was run earlier. Nothing to do here.
  //             continue;
  //         }

  //         cb.fn_(cb.arg_);
  //         cleanup_hooks_.erase(cb);
  //     }
  //     CleanupHandles();
  // }

  // for (const int fd : unmanaged_fds_)
  // {
  //     uv_fs_t close_req;
  //     uv_fs_close(nullptr, &close_req, fd, nullptr);
  //     uv_fs_req_cleanup(&close_req);
  // }
}

void Environment::RunAtExitCallbacks() {
  // TODO
  // TRACE_EVENT0(TRACING_CATEGORY_NODE1(environment), "AtExit");
  // for (ExitCallback at_exit : at_exit_functions_)
  // {
  //     at_exit.cb_(at_exit.arg_);
  // }
  // at_exit_functions_.clear();
}

void RunAtExit(Environment* env) {
  env->RunAtExitCallbacks();
}

void FreeEnvironment(Environment* env) {
  Isolate* isolate = env->isolate();
  Isolate::DisallowJavascriptExecutionScope disallow_js(
      isolate, Isolate::DisallowJavascriptExecutionScope::THROW_ON_FAILURE);
  {
    HandleScope handle_scope(isolate);  // For env->context().
    Context::Scope context_scope(env->context());
    SealHandleScope seal_handle_scope(isolate);

    env->set_stopping(true);
    // TODO
    // env->stop_sub_worker_contexts();
    env->RunCleanup();
    RunAtExit(env);
  }

  // This call needs to be made while the `Environment` is still alive
  // because we assume that it is available for async tracking in the
  // NodePlatform implementation.

  // TODO
  // MultiIsolatePlatform *platform = env->isolate_data()->platform();
  // if (platform != nullptr)
  //     platform->DrainTasks(isolate);

  delete env;
}

bool ShouldAbortOnUncaughtException(Isolate* isolate) {
  SealHandleScope scope(isolate);
  Environment* env = Environment::GetCurrent(isolate);
  return env != nullptr && (env->is_main_thread() || !env->is_stopping()) &&
         env->abort_on_uncaught_exception();
  //    env->should_abort_on_uncaught_toggle()[0] &&
  //    !env->inside_should_not_abort_on_uncaught_scope();
}

// This runs at runtime, regardless of whether the context
// is created from a snapshot.
Maybe<bool> InitializeContextRuntime(Local<Context> context) {
  // TODO
  return Just(true);
}

Maybe<bool> InitializeContextForSnapshot(Local<Context> context) {
  // Isolate *isolate = context->GetIsolate();
  // HandleScope handle_scope(isolate);

  // context->SetEmbedderData(ContextEmbedderIndex::kAllowWasmCodeGeneration,
  //                          True(isolate));

  // return InitializePrimordials(context);
}

Maybe<bool> InitializeContext(Local<Context> context) {
  // std::cout << "InitializeContext ..." << std::endl;

  // if (InitializeContextForSnapshot(context).IsNothing())
  // {
  //     return Nothing<bool>();
  // }

  return InitializeContextRuntime(context);
}

// InitializeContext, because embedders don't necessarily
// call NewContext and so they will experience breakages.
Local<Context> NewContext(Isolate* isolate) {
  auto context = Context::New(isolate, nullptr);
  if (context.IsEmpty()) {
    std::cout << "NewContext context.IsEmpty()" << std::endl;
    return context;
  }

  if (InitializeContext(context).IsNothing()) {
    return Local<Context>();
  }

  return context;
}

Maybe<bool> Environment::BootstrapPure() {
  HandleScope scope(isolate_);
  // 好吧, 我只想保持和 JavaScript 中相同的用法
  // 虽然在 C 中完全没必要这样绕个弯子来引模块
  // GetInternalBinding(this, "addon_console") = require("addon_console")
  Local<Object> __pure_stdout =
      binding::GetInternalBinding(this, "addon_console").ToLocalChecked();
  Local<Object> global = context()->Global();
  global->Set(context(),
              FIXED_ONE_BYTE_STRING(isolate_, "__pure_stdout"),
              __pure_stdout);
  global->Set(context(), FIXED_ONE_BYTE_STRING(isolate_, "global"), global)
      .Check();
  return Just(true);
}

MaybeLocal<Value> Environment::RunBootstrapping() {
  HandleScope scope(isolate_);
  std::vector<Local<String>> loaders_params = {};
  std::vector<Local<Value>> loaders_args = {};

  // 注入 addon
  BootstrapPure();
}

MaybeLocal<Value> ExecuteBootstrapper(Environment* env,
                                      const char* id,
                                      std::vector<Local<String>>* parameters,
                                      std::vector<Local<Value>>* arguments) {
  EscapableHandleScope scope(env->isolate());
  MaybeLocal<Function> maybe_fn =
      native_module::NativeModuleLoader::GetInstance()->LookupAndCompile(
          env->context(), id, parameters, env);

  Local<Function> fn;
  if (!maybe_fn.ToLocal(&fn)) {
    return MaybeLocal<Value>();
  }

  MaybeLocal<Value> result = fn->Call(env->context(),
                                      Undefined(env->isolate()),
                                      arguments->size(),
                                      arguments->data());

  return scope.EscapeMaybe(result);
}

static MaybeLocal<Value> StartExecution(Environment* env,
                                        const char* main_script_id) {
  EscapableHandleScope scope(env->isolate());
  CHECK_NOT_NULL(main_script_id);

  std::vector<Local<String>> parameters = {
      //   env->(),
      //   env->require_string(),
      //   env->internal_binding_string(),
      //   env->primordials_string(),
      //   FIXED_ONE_BYTE_STRING(env->isolate(), "markBootstrapComplete")
  };

  std::vector<Local<Value>> arguments = {
      //   env->process_object(),
      //   env->native_module_require(),
      //   env->internal_binding_loader(),
      //   env->primordials(),
      //   env->NewFunctionTemplate(MarkBootstrapComplete)
      //       ->GetFunction(env->context())
      //       .ToLocalChecked()
  };

  return scope.EscapeMaybe(
      ExecuteBootstrapper(env, main_script_id, &parameters, &arguments));
}

MaybeLocal<Value> StartExecution(Environment* env, StartExecutionCallback cb) {
  //   InternalCallbackScope callback_scope(env,
  //                                        Object::New(env->isolate()),
  //                                        {1, 0},
  //                                        InternalCallbackScope::kSkipAsyncHooks);

  if (cb != nullptr) {
    EscapableHandleScope scope(env->isolate());

    if (StartExecution(env, "internal/bootstrap/environment").IsEmpty())
      return {};

    StartExecutionCallbackInfo info = {
        // env->process_object(),
        // env->native_module_require(),
    };

    return scope.EscapeMaybe(cb(info));
  }

  StartExecution(env, "pure:console");

  return StartExecution(env, env->argv().at(1).c_str());
}

MaybeLocal<Value> LoadEnvironment(Environment* env, StartExecutionCallback cb) {
  env->InitializeLibuv();
  //   env->InitializeDiagnostics();

  return StartExecution(env, cb);
}

void Environment::InitializeLibuv() {
  HandleScope handle_scope(isolate());
  Context::Scope context_scope(context());

  CHECK_EQ(0, uv_timer_init(event_loop(), timer_handle()));
  uv_unref(reinterpret_cast<uv_handle_t*>(timer_handle()));

  CHECK_EQ(0, uv_check_init(event_loop(), immediate_check_handle()));
  uv_unref(reinterpret_cast<uv_handle_t*>(immediate_check_handle()));

  CHECK_EQ(0, uv_idle_init(event_loop(), immediate_idle_handle()));

  CHECK_EQ(0, uv_check_start(immediate_check_handle(), CheckImmediate));

  // Inform V8's CPU profiler when we're idle.  The profiler is sampling-based
  // but not all samples are created equal; mark the wall clock time spent in
  // epoll_wait() and friends so profiling tools can filter it out.  The samples
  // still end up in v8.log but with state=IDLE rather than state=EXTERNAL.
  CHECK_EQ(0, uv_prepare_init(event_loop(), &idle_prepare_handle_));
  CHECK_EQ(0, uv_check_init(event_loop(), &idle_check_handle_));

  CHECK_EQ(
      0,
      uv_async_init(event_loop(), &task_queues_async_, [](uv_async_t* async) {
        Environment* env = ContainerOf(&Environment::task_queues_async_, async);
        HandleScope handle_scope(env->isolate());
        Context::Scope context_scope(env->context());
        env->RunAndClearNativeImmediates();
      }));
  uv_unref(reinterpret_cast<uv_handle_t*>(&idle_prepare_handle_));
  uv_unref(reinterpret_cast<uv_handle_t*>(&idle_check_handle_));
  uv_unref(reinterpret_cast<uv_handle_t*>(&task_queues_async_));

  {
    Mutex::ScopedLock lock(native_immediates_threadsafe_mutex_);
    task_queues_async_initialized_ = true;
    if (native_immediates_threadsafe_.size() > 0 ||
        native_immediates_interrupts_.size() > 0) {
      uv_async_send(&task_queues_async_);
    }
  }

  // Register clean-up cb to be called to clean up the handles
  // when the environment is freed, note that they are not cleaned in
  // the one environment per process setup, but will be called in
  // FreeEnvironment.
  //   RegisterHandleCleanups();

  //   StartProfilerIdleNotifier();
}

void Environment::RunAndClearNativeImmediates(bool only_refed) {
  //   TRACE_EVENT0(TRACING_CATEGORY_NODE1(environment),
  //                "RunAndClearNativeImmediates");
  //   HandleScope handle_scope(isolate_);
  //   InternalCallbackScope cb_scope(this, Object::New(isolate_), {0, 0});

  //   size_t ref_count = 0;

  //   // Handle interrupts first. These functions are not allowed to throw
  //   // exceptions, so we do not need to handle that.
  //   RunAndClearInterrupts();

  //   auto drain_list = [&](NativeImmediateQueue* queue) {
  //     TryCatchScope try_catch(this);
  //     DebugSealHandleScope seal_handle_scope(isolate());
  //     while (auto head = queue->Shift()) {
  //       bool is_refed = head->flags() & CallbackFlags::kRefed;
  //       if (is_refed) ref_count++;

  //       if (is_refed || !only_refed) head->Call(this);

  //       head.reset();  // Destroy now so that this is also observed by
  //       try_catch.

  //       if (UNLIKELY(try_catch.HasCaught())) {
  //         if (!try_catch.HasTerminated() && can_call_into_js())
  //           errors::TriggerUncaughtException(isolate(), try_catch);

  //         return true;
  //       }
  //     }
  //     return false;
  //   };
  //   while (drain_list(&native_immediates_)) {
  //   }

  //   immediate_info()->ref_count_dec(ref_count);

  //   if (immediate_info()->ref_count() == 0) ToggleImmediateRef(false);

  //   // It is safe to check .size() first, because there is a causal
  //   relationship
  //   // between pushes to the threadsafe immediate list and this function
  //   being
  //   // called. For the common case, it's worth checking the size first before
  //   // establishing a mutex lock.
  //   // This is intentionally placed after the `ref_count` handling, because
  //   when
  //   // refed threadsafe immediates are created, they are not counted towards
  //   the
  //   // count in immediate_info() either.
  //   NativeImmediateQueue threadsafe_immediates;
  //   if (native_immediates_threadsafe_.size() > 0) {
  //     Mutex::ScopedLock lock(native_immediates_threadsafe_mutex_);
  //     threadsafe_immediates.ConcatMove(std::move(native_immediates_threadsafe_));
  //   }
  //   while (drain_list(&threadsafe_immediates)) {
  //   }
}

void Environment::CheckImmediate(uv_check_t* handle) {
  Environment* env = Environment::from_immediate_check_handle(handle);
  //   TRACE_EVENT0(TRACING_CATEGORY_NODE1(environment), "CheckImmediate");

  HandleScope scope(env->isolate());
  Context::Scope context_scope(env->context());

  env->RunAndClearNativeImmediates();

  if (env->immediate_info()->count() == 0 || !env->can_call_into_js()) return;

  Local<Object> recv;

  do {
    MakeCallback(env->isolate(),
                 recv,
                 //  env->process_object(),
                 env->immediate_callback_function(),
                 0,
                 nullptr)
        .ToLocalChecked();
  } while (env->immediate_info()->has_outstanding() && env->can_call_into_js());

  if (env->immediate_info()->ref_count() == 0) env->ToggleImmediateRef(false);
}

void Environment::ToggleImmediateRef(bool ref) {
  if (started_cleanup_) return;

  if (ref) {
    // Idle handle is needed only to stop the event loop from blocking in poll.
    uv_idle_start(immediate_idle_handle(), [](uv_idle_t*) {});
  } else {
    uv_idle_stop(immediate_idle_handle());
  }
}

void Environment::ExitEnv() {
  set_can_call_into_js(false);
  set_stopping(true);
  isolate_->TerminateExecution();
  SetImmediateThreadsafe([](Environment* env) { uv_stop(env->event_loop()); },
                         CallbackFlags::kRefed);
}
}  // namespace pure